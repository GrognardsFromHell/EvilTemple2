
#include <QSet>
#include <QString>

#include "pythonconverter.h"
#include "constants.h"

#include <Python.h>
#include <Python-ast.h>

// This is a cheap hack and must be removed if this is to be threaded!
static IConversionService *service = NULL;

class Environment {
public:
    Environment()
        : mParent(NULL)
    {
    }

    explicit Environment(Environment *parent)
        : mParent(parent)
    {
    }

    bool isParameter(const QString &variable)
    {
        if (mParameters.contains(variable))
            return true;

        if (mParent)
            return mParent->isParameter(variable);
        else
            return false;
    }

    bool isDefined(const QString &variable)
    {
        if (mLocalVariables.contains(variable))
            return true;

        if (mParent)
            return mParent->isDefined(variable);
        else
            return false;
    }

    void define(const QString &variable)
    {
        mLocalVariables.insert(variable);
    }

    void defineParameter(const QString &variable)
    {
        mParameters.insert(variable);
    }

private:
    QSet<QString> mParameters;
    QSet<QString> mLocalVariables;
    Environment *mParent;
};

PythonConverter::PythonConverter(IConversionService *service) : mService(service)
{
    Py_NoSiteFlag = 1;
    if (!Py_IsInitialized())
        Py_Initialize();
}

static void reportError()
{
    PyObject *err = PyErr_Occurred();

    if (err) {
        PyErr_Print();
    }
}

/*
 Convert an identifer to a string.
 */
QString getIdentifier(identifier id)
{
    return QString::fromLocal8Bit(PyString_AsString(id));
}

void appendIndent(int indent, QString &result)
{
    for (int i = 0; i < indent; ++i)
        result.append("    ");
}

static void convertStatement(stmt_ty stmt, QString &result, int indent, Environment *environment);
static void convertExpression(expr_ty expression, QString &result, int indent, Environment *environment, bool prefix = true);
static bool process(expr_ty expression, QString &result, int indent, Environment *environment);
static bool process(stmt_ty stmt, QString &result, int indent, Environment *environment);

static void convertName(const identifier id, const expr_context_ty ctx, QString &result, int indent, Environment *environment, bool prefix = true)
{
    QString root = QString::fromLocal8Bit(PyString_AsString(id));

    if (prefix) {
        if (!root.isNull() && !environment->isDefined(root) && !environment->isParameter(root)) {
            result.append("this.");
        }
    }

    result.append(root);
}

static QString getRootObject(expr_ty value) {
    switch (value->kind) {
    case Name_kind:
        return getIdentifier(value->v.Name.id);
    case Subscript_kind:
        return getRootObject(value->v.Subscript.value);
    case Attribute_kind:
        return getRootObject(value->v.Attribute.value);
    case Call_kind:
        return getRootObject(value->v.Call.func);
    default:
        return QString::null;
    }
}

static void convertString(const string str, QString &result, int indent)
{
    result.append("'");
    QString value = QString::fromLocal8Bit(PyString_AsString(str));
    value.replace("\\", "\\\\");
    value.replace("'", "\\'"); // TODO: This may be too simplistic
    result.append(value);
    result.append("'");
}

static void convertNumber(const object num, QString &result, int indent)
{
    if (PyFloat_Check(num)) {
        result.append(QString("%1").arg(PyFloat_AS_DOUBLE(num)));
    } else if (PyInt_Check(num)) {
        result.append(QString("%1").arg(PyInt_AS_LONG(num)));
    } else if (PyLong_Check(num)) {
        result.append(QString("%1").arg(PyLong_AsLong(num)));
    } else {
        qWarning("Unknown number type.");
    }
}

static void convertAttribute(const expr_ty value, const identifier attr, const expr_context_ty ctx, QString &result, int indent, Environment *environment)
{
    convertExpression(value, result, indent, environment);
    result.append(".");
    result.append(getIdentifier(attr));
}

static void convertCall(const expr_ty func,
                        const asdl_seq *args,
                        const asdl_seq *keywords,
                        const expr_ty starargs,
                        const expr_ty kwargs,
                        QString &result,
                        int indent, Environment *environment)
{
    if (keywords != NULL && asdl_seq_LEN(keywords) > 0)
        qWarning("Unable to handle keywords.");
    if (starargs != NULL)
        qWarning("Unable to handle starargs.");
    if (kwargs != NULL)
        qWarning("Unable to handle keyword ars.");

    convertExpression(func, result, indent, environment, true);
    result.append('(');

    for (int i = 0; i < asdl_seq_LEN(args); ++i) {
        if (i > 0)
            result.append(", ");
        convertExpression((expr_ty)asdl_seq_GET(args, i), result, indent, environment, true);
    }

    result.append(')');
}

static bool isPrimitive(expr_ty expr)
{
    return expr->kind == Str_kind ||
            expr->kind == Call_kind ||
            expr->kind == Num_kind ||
            expr->kind == Attribute_kind ||
            expr->kind == Subscript_kind ||
            expr->kind == Name_kind;
}

static void convertBinaryOp(expr_ty left,
                            operator_ty op,
                            expr_ty right,
                            QString &result,
                            int indent, Environment *environment)
{
    if (op == Pow) {
        result.append("Math.pow(");
        convertExpression(left, result, indent, environment, true);
        result.append(", ");
        convertExpression(right, result, indent, environment, true);
        result.append(")");
        return;
    }

    if (!isPrimitive(left))
        result.append("(");
    convertExpression(left, result, indent, environment, true);
    if (!isPrimitive(left))
        result.append(")");

    switch (op) {
    case Add:
        result.append(" + ");
        break;
    case Sub:
        result.append(" - ");
        break;
    case Mult:
        result.append(" * ");
        break;
    case Div:
        result.append(" / ");
        break;
    case Mod:
        result.append(" % ");
        break;
        break;
    case LShift:
        result.append(" << ");
        break;
    case RShift:
        result.append(" >> ");
        break;
    case BitOr:
        result.append(" | ");
        break;
    case BitXor:
        result.append(" ^ ");
        break;
    case BitAnd:
        result.append(" & ");
        break;
    case FloorDiv:
        qWarning("Floor div is not yet supported.");
        break;
    }

    if (!isPrimitive(right))
        result.append("(");
    convertExpression(right, result, indent, environment, true);
    if (!isPrimitive(right))
        result.append(")");
}

static void convertSubscript(expr_ty value, slice_ty slice, expr_context_ty ctx, QString &result, int indent, Environment *environment)
{

    switch (slice->kind) {
    case Index_kind:
        // This is the simple case:
        convertExpression(value, result, indent, environment);
        result.append('[');
        convertExpression(slice->v.Index.value, result, indent, environment);
        result.append(']');
        break;
    default:
        qWarning("Unsupported slice kind: %d", slice->kind);
    }
}

static void convertBoolOp(boolop_ty op, asdl_seq *values, QString &result, int indent, Environment *environment)
{
    for (int i = 0; i < asdl_seq_LEN(values); ++i) {
        if (i > 0) {
            switch (op) {
            case And:
                result.append(" && ");
                break;
            case Or:
                result.append(" || ");
                break;
            }
        }

        expr_ty value = (expr_ty)asdl_seq_GET(values, i);

        if (!isPrimitive(value) && value->kind != Compare_kind)
            result.append('(');
        convertExpression(value, result, indent, environment, true);
        if (!isPrimitive(value) && value->kind != Compare_kind)
            result.append(')');
    }
}

static void convertUnaryOp(unaryop_ty op, expr_ty operand, QString &result, int indent, Environment *environment)
{
    switch (op) {
    case Invert:
        result.append('~'); // bitwise not
        break;
    case Not:
        result.append("!"); // Boolean not
        break;
    case UAdd:
        result.append('+'); // Doesn't make much sense to me
        break;
    case USub:
        result.append('-');
        break;
    };

    if (!isPrimitive(operand))
        result.append('(');

    convertExpression(operand, result, indent, environment, true);

    if (!isPrimitive(operand))
        result.append(')');
}

static void writeCompareOperator(cmpop_ty op, QString &result)
{
    switch (op) {
    case Eq:
        result.append(" == ");
        break;
    case NotEq:
        result.append(" != ");
        break;
    case Lt:
        result.append(" < ");
        break;
    case LtE:
        result.append(" <= ");
        break;
    case Gt:
        result.append(" > ");
        break;
    case GtE:
        result.append(" >= ");
        break;
    case Is:
        qWarning("Unsupported operator: 'is'");
        break;
    case IsNot:
        qWarning("Unsupported operator: 'isnot'");
        break;
    case In:
        qWarning("Unsupported operator: 'in'");
        break;
    case NotIn:
        qWarning("Unsupported operator: 'not in'");
        break;
    }
}

static void convertComparison(expr_ty left,
                              asdl_int_seq *ops,
                              asdl_seq *comparators,
                              QString &result,
                              int indent, Environment *environment)
{
    Q_ASSERT(asdl_seq_LEN(ops) == asdl_seq_LEN(comparators));

    cmpop_ty firstOp = (cmpop_ty)asdl_seq_GET(ops, 0);

    // Special case handling for "not in" && "in"
    if (asdl_seq_LEN(ops) == 1 && (firstOp == NotIn || firstOp == In)) {
        expr_ty right = (expr_ty)asdl_seq_GET(comparators, 0);

        // Very special case of checking for a party-member
        QString rightString;
        convertExpression(right, rightString, indent, environment, true);

        /*
        This causes some trouble because we're not operating on object wrappers here.
        if (rightString == "this.game.party") {
            if (firstOp == NotIn) {
                result.append("!");
            }
            result.append("Party.isMember(");
            convertExpression(left, result, indent, environment, true);
            result.append(")");
            return;
        }*/

        result.append(rightString);
        result.append(".indexOf(");
        convertExpression(left, result, indent, environment, true);
        result.append(") ");
        if (firstOp == In)
            result.append(" != -1");
        else
            result.append(" == -1");
        return;
    }

    if (!isPrimitive(left))
        result.append('(');
    convertExpression(left, result, indent, environment, true);
    if (!isPrimitive(left))
        result.append(')');

    for (int i = 0; i < asdl_seq_LEN(comparators); ++i) {
        expr_ty value = (expr_ty)asdl_seq_GET(comparators, i);
        cmpop_ty op = (cmpop_ty)asdl_seq_GET(ops, i);

        writeCompareOperator(op, result);

        if (!isPrimitive(value))
            result.append('(');
        convertExpression(value, result, indent, environment, true);
        if (!isPrimitive(value))
            result.append(')');
    }
}

static void convertDict(asdl_seq *keys,
                        asdl_seq *values,
                        QString &result,
                        int indent, Environment *environment)
{
    if (keys == NULL || values == NULL) {
        result.append("{}"); // Empty dictionaries.
    }

    result.append("{\n");

    for (int i = 0; i < asdl_seq_LEN(keys); ++i) {
        appendIndent(indent + 1, result);
        convertExpression((expr_ty)asdl_seq_GET(keys, i), result, indent + 1, environment);
        result.append(": ");
        convertExpression((expr_ty)asdl_seq_GET(values, i), result, indent + 1, environment);
        if (i + 1 < asdl_seq_LEN(keys))
            result.append(",");
        result.append('\n');
    }

    appendIndent(indent, result);
    result.append("}");
}

static void convertList(asdl_seq *elts,
                         expr_context_ty ctx,
                         QString &result,
                         int indent, Environment *environment)
{
    result.append('[');

    for (int i = 0; i < asdl_seq_LEN(elts); ++i) {
        if (i != 0)
            result.append(", ");
        convertExpression((expr_ty)asdl_seq_GET(elts, i), result, indent, environment, true);
    }

    result.append(']');
}

static void convertTuple(asdl_seq *elts,
                         expr_context_ty ctx,
                         QString &result,
                         int indent, Environment *environment)
{
    convertList(elts, ctx, result, indent, environment);

    // JS has no tuples, so we'll use arrays instead. This is very problematic for assignments though.
    if (ctx != 1) {
        QString debugText;
        convertList(elts, ctx, debugText, indent, environment);

        qWarning("Converting tuple with ctx %d: %s", ctx, qPrintable(debugText));
    }
}

static void convertExpression(const expr_ty expression, QString &result, int indent, Environment *environment, bool prefix)
{
    /**
      Check if the expression has a special meaning and must be converted to a special-case instruction.
      I.e. legacy quest-state setting / D20 dice rolls
      */
    if (process(expression, result, indent, environment))
        return;

    switch (expression->kind) {
    case BoolOp_kind:
        convertBoolOp(expression->v.BoolOp.op,
                        expression->v.BoolOp.values,
                        result,
                        indent, environment);
        break;
    case BinOp_kind:
        convertBinaryOp(expression->v.BinOp.left,
                        expression->v.BinOp.op,
                        expression->v.BinOp.right,
                        result,
                        indent, environment);
        break;
    case UnaryOp_kind:
        convertUnaryOp(expression->v.UnaryOp.op,
                       expression->v.UnaryOp.operand,
                       result,
                       indent, environment);
        break;
    case Lambda_kind:
        qWarning("Lambdas are not supported.");
        break;
    case IfExp_kind:
        qWarning("If Expressions are not supported");
        break;
    case Dict_kind:
        convertDict(expression->v.Dict.keys,
                    expression->v.Dict.values,
                    result,
                    indent, environment);
        break;
    case ListComp_kind:
        break;
    case GeneratorExp_kind:
        qWarning("Generators are not supported.");
        break;
    case Yield_kind:
        qWarning("Yield is not supported.");
        break;
    case Compare_kind:
        convertComparison(expression->v.Compare.left,
                          expression->v.Compare.ops,
                          expression->v.Compare.comparators,
                          result,
                          indent, environment);
        break;
    case Call_kind:
        convertCall(expression->v.Call.func,
                    expression->v.Call.args,
                    expression->v.Call.keywords,
                    expression->v.Call.starargs,
                    expression->v.Call.kwargs,
                    result,
                    indent, environment);
        break;
    case Repr_kind:
        qWarning("Repr is not supported.");
        break;
    case Num_kind:
        convertNumber(expression->v.Num.n, result, indent);
        break;
    case Str_kind:
        convertString(expression->v.Str.s, result, indent);
        break;
    case Attribute_kind:
        convertAttribute(expression->v.Attribute.value, expression->v.Attribute.attr, expression->v.Attribute.ctx,
                         result, indent, environment);
        break;
    case Subscript_kind:
        convertSubscript(expression->v.Subscript.value,
                         expression->v.Subscript.slice,
                         expression->v.Subscript.ctx,
                         result,
                         indent, environment);
        break;
    case Name_kind:
        convertName(expression->v.Name.id, expression->v.Name.ctx, result, indent, environment, prefix);
        break;
    case List_kind:
        convertList(expression->v.List.elts,
                     expression->v.List.ctx,
                     result,
                     indent, environment);
        break;
    case Tuple_kind:
        convertTuple(expression->v.Tuple.elts,
                     expression->v.Tuple.ctx,
                     result,
                     indent, environment);
        break;
    }
}

static void convertFunction(const _stmt *stmt, QString &result, int indent, Environment *parentEnvironment)
{
    Environment localEnvironment(parentEnvironment);

    appendIndent(indent, result);

    QString name = getIdentifier(stmt->v.FunctionDef.name);

    result.append("function ").append(name).append("(");

    // convert args
    const arguments_ty args = stmt->v.FunctionDef.args;

    if (args->defaults) {
        qWarning("Don't know how to handle defaults for function %s.", qPrintable(name));
    }

    for (int i = 0; i < asdl_seq_LEN(args->args); ++i) {
        const expr_ty name = (expr_ty)asdl_seq_GET(args->args, i);

        if (i > 0)
            result.append(", ");

        if (name->kind == Name_kind) {
            localEnvironment.defineParameter(getIdentifier(name->v.Name.id));
        } else {
            qWarning("Function argument declaration that is not a name: %d.", name->kind);
        }

        convertExpression(name, result, indent, &localEnvironment);
    }

    result.append(") {\n");

    // convert body

    for (int i = 0; i < asdl_seq_LEN(stmt->v.FunctionDef.body); ++i) {
        convertStatement((stmt_ty)asdl_seq_GET(stmt->v.FunctionDef.body, i), result, indent + 1, &localEnvironment);
    }

    result.append("}\n\n");
}

static void convertRootFunction(const _stmt *stmt, QString &result, int indent, Environment *parentEnvironment, bool last)
{
    Environment localEnvironment(parentEnvironment);

    QString name = getIdentifier(stmt->v.FunctionDef.name);

    appendIndent(indent, result);
    result.append(name);
    result.append(": function (");

    // convert args
    const arguments_ty args = stmt->v.FunctionDef.args;

    if (args->defaults) {
        qWarning("Don't know how to handle defaults for function %s.", qPrintable(name));
    }

    for (int i = 0; i < asdl_seq_LEN(args->args); ++i) {
        const expr_ty name = (expr_ty)asdl_seq_GET(args->args, i);

        if (name->kind == Name_kind) {
            localEnvironment.define(getIdentifier(name->v.Name.id));
        } else {
            qWarning("Function argument declaration that is not a name: %d.", name->kind);
        }

        if (i > 0)
            result.append(", ");

        convertExpression(name, result, indent, &localEnvironment);
    }

    result.append(") {\n");

    // convert body

    for (int i = 0; i < asdl_seq_LEN(stmt->v.FunctionDef.body); ++i) {
        convertStatement((stmt_ty)asdl_seq_GET(stmt->v.FunctionDef.body, i), result, indent + 1, &localEnvironment);
    }

    appendIndent(indent, result);
    if (last)
        result.append("}\n");
    else
        result.append("},\n");
}

static void convertPrint(expr_ty dest, asdl_seq *values, bool nl, QString &result, int indent, Environment *environment)
{
    appendIndent(indent, result);
    result.append("print(");

    for (int i = 0; i < asdl_seq_LEN(values); ++i) {
        if (i > 0)
            result.append(" + ");
        convertExpression((expr_ty)asdl_seq_GET(values, i), result, indent, environment);
    }

    result.append(");\n");
}

static void convertRootAssignment(asdl_seq *targets, expr_ty value, QString &result, int indent, Environment *environment, bool last)
{
    int len = asdl_seq_LEN(targets);
    for (int i = 0; i < len; ++i) {
        expr_ty target = (expr_ty)asdl_seq_GET(targets, i);

        appendIndent(indent, result);

        // Check if targets is a name, then check if it's already defined
        if (target->kind != Name_kind) {
            qWarning("Root assignment to something other than a name!");
        }

        convertExpression(target, result, indent, environment);
        result.append(": ");
        convertExpression(value, result, indent, environment);
        if (!last || i + 1 < len)
            result.append(",\n");
        else
            result.append("\n");
    }

    // Give it some extra space
    result.append('\n');
}

static void convertAssignment(asdl_seq *targets, expr_ty value, QString &result, int indent, Environment *environment)
{
    appendIndent(indent, result);

    expr_ty target = (expr_ty)asdl_seq_GET(targets, 0);

    // Check if targets is a name, then check if it's already defined
    if (target->kind == Name_kind) {
        QString name = getIdentifier(target->v.Name.id);
        if (!environment->isDefined(name) && !environment->isParameter(name)) {
            environment->define(name);
            result.append("var ");
        }
    }

    convertExpression(target, result, indent, environment, true);
    result.append(" = ");
    convertExpression(value, result, indent, environment, true);
    result.append(";\n");

    for (int i = 1; i < asdl_seq_LEN(targets); ++i) {
        target = (expr_ty)asdl_seq_GET(targets, i);

        appendIndent(indent, result);

        // Check if targets is a name, then check if it's already defined
        if (target->kind == Name_kind) {
            QString name = getIdentifier(target->v.Name.id);
            if (!environment->isDefined(name) && !environment->isParameter(name)) {
                environment->define(name);
                result.append("var ");
            }
        }

        convertExpression(target, result, indent, environment, true);
        result.append(" = ");
        convertExpression((expr_ty)asdl_seq_GET(targets, 0), result, indent, environment, true);
        result.append(";\n");
    }

    // Give it some extra space
    result.append('\n');
}

static void convertAugAssignment(expr_ty target, operator_ty op, expr_ty value, QString &result, int indent, Environment *environment)
{

    appendIndent(indent, result);
    convertExpression(target, result, indent, environment, true);
    switch (op) {
    case Add:
        result.append(" += ");
        break;
    case Sub:
        result.append(" -= ");
        break;
    case Mult:
        result.append(" *= ");
        break;
    case FloorDiv:
        qWarning("Floor div not supported.");
    case Div:
        result.append(" /= ");
        break;
    case Mod:
        result.append(" %= ");
        break;
    case LShift:
        result.append(" <<= ");
        break;
    case RShift:
        result.append(" >>= ");
        break;
    case BitOr:
        result.append(" |= ");
        break;
    case BitXor:
        result.append(" ^= ");
        break;
    case BitAnd:
        result.append(" &= ");
        break;
    default:
        qWarning("Unsupported operator for inplace assignment: %d", op);
    }
    convertExpression(value, result, indent, environment, true);
    result.append(";\n");

}

static void convertIfElse(expr_ty test,
                          asdl_seq *body,
                          asdl_seq *orelse,
                          QString &result,
                          int indent,
                          Environment *environment,
                          bool elseIf = false)
{
    if (!elseIf)
        appendIndent(indent, result);
    result.append("if (");
    convertExpression(test, result, indent + 1, environment, true);
    result.append(") {\n");

    for (int i = 0; i < asdl_seq_LEN(body); ++i) {
        convertStatement((stmt_ty)asdl_seq_GET(body, i), result, indent + 1, environment);
    }

    bool elseBranch = false;

    for (int i = 0; i < asdl_seq_LEN(orelse); ++i) {
        stmt_ty branch = (stmt_ty)asdl_seq_GET(orelse, i);

        // Else-If branch
        if (branch->kind == If_kind && !elseBranch) {
            appendIndent(indent, result);
            result.append("} else ");
            convertIfElse(branch->v.If.test, branch->v.If.body, branch->v.If.orelse, result, indent, environment, true);
        } else {
            if (!elseBranch) {
                appendIndent(indent, result);
                result.append("} else {\n");
                elseBranch = true;
            }
            convertStatement(branch, result, indent + 1, environment);
        }
    }

    if (!elseIf) {
        // convert else branches
        appendIndent(indent, result);
        result.append("}\n");
    }
}

static void convertReturn(expr_ty value, QString &result, int indent, Environment *environment)
{
    result.append("\n");
    appendIndent(indent, result);
    if (value != NULL) {
        result.append("return ");
        convertExpression(value, result, indent, environment, true);
    } else {
        result.append("return");
    }
    result.append(";\n");
}

static void convertFor(expr_ty target,
                       expr_ty iter,
                       asdl_seq *body,
                       asdl_seq *orelse,
                       QString &result,
                       int indent,
                       Environment *environment)
{

    // The actual iterator depends on what we're iterating on. If we're iterating over "i", we use "foreach"

    QString iterateOn = "objects";
    int append = 1;
    while (environment->isDefined(iterateOn)) {
        iterateOn = QString("objects%1").arg(append++);
    }

    appendIndent(indent, result);
    result.append("var ").append(iterateOn).append(" = ");
    convertExpression(iter, result, indent, environment, true);
    result.append(";\n");

    QString iterator = "i";
    append = 1;
    while (environment->isDefined(iterator)) {
        iterator= QString("i%1").arg(append++);
    }

    appendIndent(indent, result);
    result.append(QString("for (var %1 = 0; %1 < %2.length; ++%1) {\n").arg(iterator).arg(iterateOn));
    appendIndent(indent + 1, result);

    Environment localEnvironment(environment);
    if (target->kind == Name_kind) {
        localEnvironment.defineParameter(getIdentifier(target->v.Name.id));
    }

    result.append("var ");
    convertExpression(target, result, indent, &localEnvironment);
    result.append(QString(" = %1[%2];\n").arg(iterateOn).arg(iterator));

    for (int i = 0; i < asdl_seq_LEN(body); ++i) {
        convertStatement((stmt_ty)asdl_seq_GET(body, i), result, indent + 1, &localEnvironment);
    }

    appendIndent(indent, result);
    result.append("}\n\n");

    if (asdl_seq_LEN(orelse) != 0) {
        qWarning("For loops with else branches are unsupported.");
    }
}

static void convertWhile(expr_ty test,
                       asdl_seq *body,
                       asdl_seq *orelse,
                       QString &result,
                       int indent,
                       Environment *environment)
{

    // The actual iterator depends on what we're iterating on. If we're iterating over "i", we use "foreach"

    appendIndent(indent, result);
    result.append("while (");
    convertExpression(test, result, indent, environment, true);
    result.append(") {\n");

    for (int i = 0; i < asdl_seq_LEN(body); ++i) {
        convertStatement((stmt_ty)asdl_seq_GET(body, i), result, indent + 1, environment);
    }

    appendIndent(indent, result);
    result.append("}\n");

    if (asdl_seq_LEN(orelse) != 0) {
        qWarning("While loops with else branches are unsupported.");
    }
}

static void convertDelete(asdl_seq *targets,
                          QString &result,
                          int indent,
                          Environment *environment)
{

    if (asdl_seq_LEN(targets) == 0) {
        qDebug("NO TARGETS");
    }

    for (int i = 0; i < asdl_seq_LEN(targets); ++i) {
        expr_ty target = (expr_ty)asdl_seq_GET(targets, i);

        appendIndent(indent, result);
        result.append("delete ");
        convertExpression(target, result, indent, environment, true);
        result.append(";\n");
    }
}

static void convertStatement(stmt_ty stmt, QString &result, int indent, Environment *environment)
{
    if (process(stmt, result, indent, environment))
        return;

    switch (stmt->kind) {
    case FunctionDef_kind:
        convertFunction(stmt, result, indent, environment); // Nested functions get a new environment
        break;

    case ClassDef_kind:
        qWarning("Class definitions are not supported.");
        break;

    case Return_kind:
        if (indent == 0) {
            qWarning("Top-Level return statement");
        }
        convertReturn(stmt->v.Return.value, result, indent, environment);
        break;

    case Delete_kind:
        convertDelete(stmt->v.Delete.targets,
                      result,
                      indent,
                      environment);
        break;

    case Assign_kind:
        convertAssignment(stmt->v.Assign.targets, stmt->v.Assign.value, result, indent, environment);
        break;

    case AugAssign_kind:
        convertAugAssignment(stmt->v.AugAssign.target, stmt->v.AugAssign.op, stmt->v.AugAssign.value, result, indent, environment);
        break;

    case Print_kind:
        if (indent == 0) {
            qWarning("Top-Level print statement");
        }
        convertPrint(stmt->v.Print.dest, stmt->v.Print.values, stmt->v.Print.nl, result, indent, environment);
        break;

    case For_kind:
        if (indent == 0) {
            qWarning("Top-Level for statement");
        }
        convertFor(stmt->v.For.target,
                   stmt->v.For.iter,
                   stmt->v.For.body,
                   stmt->v.For.orelse,
                   result,
                   indent,
                   environment);
        break;

    case While_kind:
        if (indent == 0) {
            qWarning("Top-Level while statement");
        }
        convertWhile(stmt->v.While.test,
                     stmt->v.While.body,
                     stmt->v.While.orelse,
                     result,
                     indent,
                     environment);
        break;

    case If_kind:
        if (indent == 0) {
            qWarning("Top-Level if statement");
        }
        convertIfElse(stmt->v.If.test,
                      stmt->v.If.body,
                      stmt->v.If.orelse,
                      result,
                      indent,
                      environment);
        break;

    case With_kind:
        qWarning("With is not yet supported.");
        break;

    case Raise_kind:
        qWarning("Exceptions are not yet supported.");
        break;

    case TryExcept_kind:
        qWarning("Exceptions are not yet supported.");
        break;

    case TryFinally_kind:
        qWarning("Exceptions are not yet supported.");
        break;

    case Assert_kind:
        qWarning("Asserts are not yet supported.");
        break;

    case Import_kind:
        break;

    case ImportFrom_kind:
        break;

    case Exec_kind:
        qWarning("Exec is not yet supported.");
        break;

    case Global_kind:
        qWarning("Global is not yet supported.");
        break;

    case Expr_kind:
        appendIndent(indent, result);
        convertExpression(stmt->v.Expr.value, result, indent + 1, environment, true);
        result.append(";\n");
        break;

    case Pass_kind:
        break;

    case Break_kind:
        if (indent == 0) {
            qWarning("Top-Level break statement");
        }
        appendIndent(indent, result);
        result.append("break;\n");
        break;

    case Continue_kind:
        if (indent == 0) {
            qWarning("Top-Level continue statement");
        }
        appendIndent(indent, result);
        result.append("continue;\n");
        break;
    }
}

static void convertRootStatement(const _stmt *stmt, QString &result, int indent, Environment *environment, bool last)
{
    switch (stmt->kind) {
    case FunctionDef_kind:
        convertRootFunction(stmt, result, indent, environment, last); // Nested functions get a new environment
        break;

    case Assign_kind:
        convertRootAssignment(stmt->v.Assign.targets, stmt->v.Assign.value, result, indent, environment, last);
        break;

    // Should this lead to real symbol imports?
    case ImportFrom_kind:
    case Import_kind:
        break;

    default:
        qWarning("Invalid root statement type: %d", stmt->kind);
    }
}

static void dumpModule(const _mod *module, QString &result)
{
    int len;

    Environment environment;

    result.append("{\n");

    switch (module->kind) {
    case Module_kind:
        len = asdl_seq_LEN(module->v.Module.body);
        for (int i = 0; i < len; ++i)
            convertRootStatement((_stmt*)asdl_seq_GET(module->v.Module.body, i), result, 1, &environment,
                                 i + 1 >= len);
        break;
    case Interactive_kind:
        qFatal("interactive");
        break;
    case Expression_kind:
        qFatal("expression");
        break;
    case Suite_kind:
        qFatal("suite");
        break;
    }

    result.append("}\n");
}

QString PythonConverter::convert(const QByteArray &code, const QString &filename)
{
    service = mService;

    QString result;

    PyArena *arena = PyArena_New();

    if (!arena) {
        qFatal("Unable to create arena for python conversion.");
    }

    QByteArray textCode = code;
    textCode.replace("\r", "");
    textCode.replace("else if", "elif"); // Fixes some ToEE bugs
    textCode = textCode.trimmed();

    PyCompilerFlags flags;
    flags.cf_flags = PyCF_SOURCE_IS_UTF8|PyCF_ONLY_AST;

    _mod *astRoot = PyParser_ASTFromString(textCode.constData(), qPrintable(filename), Py_file_input, &flags, arena);

    if (!astRoot) {
        PyErr_Print();
        qWarning("Unable to parse python file: %s.", qPrintable(filename));
        return result;
    }

    dumpModule(astRoot, result);

    PyArena_Free(arena);

    return result;
}

QString PythonConverter::convertDialogGuard(const QByteArray &code, const QString &filename)
{
    service = mService;

    QString result;

    PyArena *arena = PyArena_New();

    if (!arena) {
        qFatal("Unable to create arena for python conversion.");
    }

    QByteArray textCode = code;
    textCode.replace("\r", "");
    textCode.replace(" = ", " == "); // This fixes some issues. dialog guards should be read-only anyway
    textCode.replace(" nd ", " and "); // Another fix for sloppy Troika QA
    textCode = textCode.trimmed();

    PyCompilerFlags flags;
    flags.cf_flags = PyCF_SOURCE_IS_UTF8|PyCF_ONLY_AST;

    _mod *astRoot = PyParser_ASTFromString(textCode.constData(), qPrintable(filename), Py_eval_input, &flags, arena);

    if (!astRoot) {
        PyErr_Print();
        qWarning("Unable to parse python file: %s.", qPrintable(filename));
        return result;
    }

    Environment environment;
    environment.defineParameter("npc");
    environment.defineParameter("pc");

    switch (astRoot->kind) {
    case Expression_kind:
        convertExpression(astRoot->v.Expression.body, result, 0, &environment, true);
        break;
    default:
        qFatal("Invalid dialog guard AST type: %d", astRoot->kind);
        break;
    }

    PyArena_Free(arena);

    return result;
}

QString PythonConverter::convertDialogAction(const QByteArray &code, const QString &filename)
{
    service = mService;

    QString result;

    PyArena *arena = PyArena_New();

    if (!arena) {
        qFatal("Unable to create arena for python conversion.");
    }

    QList<QByteArray> lines = code.split(';'); // Lines are separated by semicolons in dlg actions
    QByteArray textCode;

    foreach (const QByteArray &line, lines) {
        textCode.append(line.trimmed()).append("\n");
    }

    PyCompilerFlags flags;
    flags.cf_flags = PyCF_SOURCE_IS_UTF8|PyCF_ONLY_AST;

    _mod *astRoot = PyParser_ASTFromString(textCode.constData(), qPrintable(filename), Py_file_input, &flags, arena);

    if (!astRoot) {
        PyErr_Print();
        qWarning("Unable to parse python file: %s.", qPrintable(filename));
        return result;
    }

    int len;

    Environment environment;
    environment.defineParameter("npc");
    environment.defineParameter("pc");

    switch (astRoot->kind) {
    case Module_kind:
        len = asdl_seq_LEN(astRoot->v.Module.body);
        for (int i = 0; i < len; ++i)
            convertStatement((stmt_ty)asdl_seq_GET(astRoot->v.Module.body, i), result, 0, &environment);
        break;
    case Suite_kind:
        qFatal("Invalid AST type for dialog action: %d", astRoot->kind);
        break;
    }

    PyArena_Free(arena);

    return result.trimmed();
}

inline static bool isIdentifier(expr_ty expression, const QString &id) {
    if (expression->kind != Name_kind)
        return false;

    return getIdentifier(expression->v.Name.id) == id;
}

inline static bool isObjectHandleNull(expr_ty expression)
{
    return isIdentifier(expression, "OBJ_HANDLE_NULL");
}

inline static bool isGameObject(expr_ty expression)
{
    return isIdentifier(expression, "game");
}

static bool isQuestStateField(expr_ty expression, QString *questId, Environment *environment)
{
    /*
     This is actually optional. Some scripts refer to the quest state
     either via game.quests[id].state or game.quests[id] directly.
     */
    if (expression->kind == Attribute_kind) {
        QString attribName = getIdentifier(expression->v.Attribute.attr);

        if (attribName != "state")
            return false;

        expression = expression->v.Attribute.value;
    }

    if (expression->kind != Subscript_kind)
        return false;

    expr_ty value = expression->v.Subscript.value;

    if (value->kind != Attribute_kind
        || getIdentifier(value->v.Attribute.attr) != "quests"
        || !isGameObject(value->v.Attribute.value))
        return false;

    slice_ty slice = expression->v.Subscript.slice;

    if (slice->kind != Index_kind)
        return false;

    convertExpression(slice->v.Index.value, *questId, 0, environment, true);
    return true;
}

static bool isGlobalFlagsField(expr_ty expression, QString *flagId, Environment *environment)
{
    if (expression->kind != Subscript_kind)
        return false;

    expr_ty value = expression->v.Subscript.value;

    if (value->kind != Attribute_kind
        || getIdentifier(value->v.Attribute.attr) != "global_flags"
        || !isGameObject(value->v.Attribute.value))
        return false;

    slice_ty slice = expression->v.Subscript.slice;

    if (slice->kind != Index_kind)
        return false;

    convertExpression(slice->v.Index.value, *flagId, 0, environment, true);
    return true;
}

static bool isAreaProperty(expr_ty expression)
{
    return (expression->kind == Attribute_kind && getIdentifier(expression->v.Attribute.attr) == "area");
}

static bool isMapProperty(expr_ty expression)
{
    return (expression->kind == Attribute_kind && getIdentifier(expression->v.Attribute.attr) == "map");
}

static bool isGlobalVarsField(expr_ty expression, QString *varId, Environment *environment)
{
    if (expression->kind != Subscript_kind)
        return false;

    expr_ty value = expression->v.Subscript.value;

    if (value->kind != Attribute_kind
        || getIdentifier(value->v.Attribute.attr) != "global_vars"
        || !isGameObject(value->v.Attribute.value))
        return false;

    slice_ty slice = expression->v.Subscript.slice;

    if (slice->kind != Index_kind)
        return false;

    convertExpression(slice->v.Index.value, *varId, 0, environment, true);
    return true;
}

static void processQuestId(QString &questId) {
    bool ok;
    uint questIdNum = questId.toUInt(&ok);
    if (ok) {
        questId = QString("'quest-%1'").arg(questIdNum);
        return;
    }

    questId = "'quest-' + " + questId; // Treat as identifier
}

static bool isAreasField(expr_ty expression, QString *mapId, Environment *environment)
{
    if (expression->kind != Subscript_kind)
        return false;

    expr_ty value = expression->v.Subscript.value;

    if (value->kind != Attribute_kind
        || getIdentifier(value->v.Attribute.attr) != "areas"
        || !isGameObject(value->v.Attribute.value))
        return false;

    slice_ty slice = expression->v.Subscript.slice;

    if (slice->kind != Index_kind)
        return false;

    convertExpression(slice->v.Index.value, *mapId, 0, environment, true);
    return true;
}

/**
  Used to convert comparisons of the form 'npc.area == 1' to symbolic constants.
  */
static QString areaIds[] = {
    "",
    "Area.Hommlet",
    "Area.Moathouse",
    "Area.Nulb",
    "Area.Temple",
    "Area.EmridyMeadows",
    "Area.ImerydsRun",
    "Area.TempleSecretExit",
    "Area.MoathouseSecretExit",
    "Area.OgreCave",
    "Area.DekloGrove",
    "Area.TempleRuinedHouse",
    "Area.TempleTower"
};

static int expressionToInt(expr_ty expression, bool *ok = NULL)
{
    if (expression->kind != Num_kind) {
        if (ok)
            *ok = false;
        return 0;
    }

    object numObj = expression->v.Num.n;

    if (PyInt_Check(numObj)) {
        if (ok)
            *ok = true;
        return PyInt_AS_LONG(numObj);
    } else if (PyLong_Check(numObj)) {
        if (ok)
            *ok = true;
        return PyLong_AsLong(numObj);
    } else {
        if (ok)
            *ok = false;
        return 0;
    }
}

/*
   These are pre-processing functions that operate on the AST level to convert several commonly used
   idioms from ToEE to a more modern version.
 */
static bool process(expr_ty expression, QString &result, int indent, Environment *environment)
{
    if (expression->kind == Call_kind) {
        QString func;
        convertExpression(expression->v.Call.func, func, 0, environment);

        asdl_seq *args = expression->v.Call.args;

        /*
         Convert calls to "dice_new" into constructor calls for Dice.
         */
        if (func == "this.dice_new") {
            result.append("new Dice(");
            for (int i = 0; i < asdl_seq_LEN(args); ++i) {
                if (i != 0)
                    result.append(", ");
                convertExpression((expr_ty)asdl_seq_GET(args, i), result, indent, environment, true);
            }
            result.append(")");
            return true;
        } else if (func == "this.game.random_range") {
            result.append("randomRange(");
            for (int i = 0; i < asdl_seq_LEN(args); ++i) {
                if (i != 0)
                    result.append(", ");
                convertExpression((expr_ty)asdl_seq_GET(args, i), result, indent, environment, true);
            }
            result.append(")");
            return true;
        } else if (func == "this.game.fade_and_teleport") {
            result.append("this.game.fade_and_teleport(");
            for (int i = 0; i < asdl_seq_LEN(args); ++i) {
                if (i != 0)
                    result.append(", ");
                if (i == 3) {
                    // Convert the map id string
                    QString newMapId = service->convertMapId(expressionToInt((expr_ty)asdl_seq_GET(args, i)));
                    if (newMapId.isNull()) {
                        result.append("'invalid-map'");
                        qWarning("Invalid map id to fade_and_teleport.");
                    } else {
                        result.append("'" + newMapId + "'");
                    }
                } else if (i == 4 || i == 5) {
                    // Convert the coordinates to the new system by multiplying with PixelPerWorldTile
                    bool ok;
                    int coord = expressionToInt((expr_ty)asdl_seq_GET(args, i), &ok);
                    if (!ok) {
                        qWarning("Invalid parameter to fade_and_teleport.");
                    }
                    result.append(QString("%1").arg((int)((coord + 0.5f) * PixelPerWorldTile)));
                } else {
                    convertExpression((expr_ty)asdl_seq_GET(args, i), result, indent, environment, true);
                }
            }
            result.append(")");
            return true;
        } else if (func == "this.game.map_flags") {
            result.append("this.game.map_flags(");
            for (int i = 0; i < asdl_seq_LEN(args); ++i) {
                if (i != 0)
                    result.append(", ");
                if (i == 0) {
                    // Convert the map id string
                    QString mapIdString;
                    convertExpression((expr_ty)asdl_seq_GET(args, i), mapIdString, indent, environment, true);
                    bool ok;
                    uint mapId = mapIdString.toUInt(&ok);
                    QString newMapId = service->convertMapId(mapId);
                    if (!ok || newMapId.isNull()) {
                        result.append("'invalid-map'");
                        qWarning("Invalid map id : %s", mapIdString);
                    } else {
                        result.append("'" + newMapId + "'");
                    }
                } else {
                    convertExpression((expr_ty)asdl_seq_GET(args, i), result, indent, environment, true);
                }
            }
            result.append(")");
            return true;
        }

    } else if (expression->kind == Name_kind) {
        QString name = getIdentifier(expression->v.Name.id);

        /*
         Convert OBJ_HANDLE_NULL to null. Wonder why they didn't use undef...?
         */
        if (name == "OBJ_HANDLE_NULL") {
            result.append("null");
            return true;
        } else if (name == "LAWFUL_GOOD") {
            result.append("Alignment.LawfulGood");
            return true;
        } else if (name == "NEUTRAL_GOOD") {
            result.append("Alignment.NeutralGood");
            return true;
        } else if (name == "CHAOTIC_GOOD") {
            result.append("Alignment.ChaoticGood");
            return true;
        } else if (name == "LAWFUL_NEUTRAL") {
            result.append("Alignment.LawfulNeutral");
            return true;
        } else if (name == "TRUE_NEUTRAL") {
            result.append("Alignment.TrueNeutral");
            return true;
        } else if (name == "CHAOTIC_NEUTRAL") {
            result.append("Alignment.ChaoticNeutral");
            return true;
        } else if (name == "LAWFUL_EVIL") {
            result.append("Alignment.LawfulEvil");
            return true;
        } else if (name == "NEUTRAL_EVIL") {
            result.append("Alignment.NeutralEvil");
            return true;
        } else if (name == "CHAOTIC_EVIL") {
            result.append("Alignment.ChaoticEvil");
            return true;
        }
    } else if (expression->kind == Subscript_kind) {
        QString varId;
        if (isGlobalVarsField(expression, &varId, environment)) {
            result.append("GlobalVars.get(").append(varId).append(")");
            return true;
        }

    } else if (expression->kind == Attribute_kind) {
        expr_ty value = expression->v.Attribute.value;
        QString attr = getIdentifier(expression->v.Attribute.attr);

        if (attr == "story_state" && isGameObject(value)) {
            result.append("StoryState");
            return true;
        } else if (attr == "party_alignment" && isGameObject(value)) {
            result.append("Party.alignment");
            return true;
        } else if (attr == "location") {
            convertExpression(value, result, indent, environment, true);
            result.append(".position");
            return true;
        } else if (attr == "name") {
            convertExpression(value, result, indent, environment, true);
            result.append(".internalId");
            return true;
        }/*
            This causes some trouble because we're not operating on object wrappers.
            else if (attr == "party" && isGameObject(value)) {
            result.append("Party.getMembers()");
            return true;
        }*/

    } else if (expression->kind == Compare_kind) {
        asdl_int_seq *ops = expression->v.Compare.ops;

        if (asdl_seq_LEN(ops) > 1) {
            qWarning("Skipping unsupported multi-operator comparison.");
            return false; // Don't handle multi-op compares
        }

        cmpop_ty op = (cmpop_ty)asdl_seq_GET(ops, 0);
        expr_ty left = expression->v.Compare.left;
        expr_ty right = (expr_ty)asdl_seq_GET(expression->v.Compare.comparators, 0);

        if (op == NotEq) {
            /*
             Comparisons against OBJ_HANDLE_NULL should be converted to direct checks for the object.
             */
            if (isObjectHandleNull(right)) {
                convertExpression(left, result, 0, environment, true);
                return true;
            } else if (isObjectHandleNull(left)) {
                convertExpression(right, result, 0, environment, true);
                return true;
            }
        }

        /*
         Querying the .area field in a comparison will convert the constant operand to a string
         corresponding to the new area ids.
         */
        if (isAreaProperty(left)) {
            // Evaluate the right hand side
            QString areaIdString;
            convertExpression(right, areaIdString, 0, environment, false);
            bool ok;
            uint areaId = areaIdString.toUInt(&ok);

            if (!ok || areaId == 0 || areaId > 12) {
                qDebug("Unknown area id encountered: %s", qPrintable(areaIdString));

                convertExpression(left, result, indent, environment, true);
                writeCompareOperator(op, result);
                result.append("'area-").append(areaIdString).append("'");
                return true;
            } else {
                convertExpression(left, result, indent, environment, true);
                writeCompareOperator(op, result);
                result.append(areaIds[areaId]);
                return true;
            }

        } else if (isAreaProperty(right)) {
            qWarning("Didn't account for npc.area to be on the right hand side.");
        }


        /*
         Querying the .map field of an NPC or PC will convert the right-hand-side from the numeric
         id to the new string ids.
         */
        if (isMapProperty(left)) {

            // TODO: if the LHS is game.leader, rewrite to Maps.currentMap

            // Evaluate the right hand side
            QString mapIdString;
            convertExpression(right, mapIdString, 0, environment, false);
            bool ok;
            uint mapId = mapIdString.toUInt(&ok);
            QString mapConstant = service->convertMapId(mapId);

            if (!ok || mapConstant.isNull()) {
                qWarning("Invalid map id encountered: %s", qPrintable(mapIdString));
            } else {
                convertExpression(left, result, indent, environment, true);
                writeCompareOperator(op, result);
                result.append("'");
                result.append(mapConstant);
                result.append("'");
                return true;
            }

        } else if (isMapProperty(right)) {
            qWarning("Didn't account for npc.map to be on the right hand side.");
        }

        /*
         Quests state queries should be redirected to the new system.
         i.e.: game.quests[18].state == this.qs_unknown ==> Quests.isUnknown(18)
         */
        QString questId;
        if (isQuestStateField(left, &questId, environment)) {
            processQuestId(questId);

            /*
                Check against which constant the quest state is compared.
                We also account for some misspellings in the original scripts here:
                qs_acccepted,
            */
            if (op == Eq || op == NotEq) {
                if (op == NotEq)
                    result.append('!'); // Negate the result of equality here

                if (isIdentifier(right, "qs_unknown")) {
                    result.append(QString("Quests.isUnknown(%1)").arg(questId));
                } else if (isIdentifier(right, "qs_mentioned")) {
                    result.append(QString("Quests.isMentioned(%1)").arg(questId));
                } else if (isIdentifier(right, "qs_accepted") || isIdentifier(right, "qs_acccepted")) {
                    result.append(QString("Quests.isAccepted(%1)").arg(questId));
                } else if (isIdentifier(right, "qs_completed")) {
                    result.append(QString("Quests.isCompleted(%1)").arg(questId));
                } else if (isIdentifier(right, "qs_botched")) {
                    result.append(QString("Quests.isBotched(%1)").arg(questId));
                } else {
                    QString value;
                    convertExpression(right, value, 0, environment, false);
                    qWarning("Comparing (EQ,NOTEQ) quest state to unknown value: %s", qPrintable(value));
                }
            } else if (op == LtE) {
                result.append('!');

                if (isIdentifier(right, "qs_mentioned")) {
                    // qs <= mentioned means the quest hasn't been started yet
                    result.append(QString("Quests.isStarted(%1)").arg(questId));
                } else if (isIdentifier(right, "qs_accepted") || isIdentifier(right, "qs_acccepted")) {
                    // qs <= qs_accepted means the quest hasn't been finished yet
                    result.append(QString("Quests.isFinished(%1)").arg(questId));
                } else {
                    QString value;
                    convertExpression(right, value, 0, environment, false);
                    qWarning("Comparing (LTE) quest state to unknown value: %s", qPrintable(value));
                }
            } else if (op == GtE) {

                if (isIdentifier(right, "qs_mentioned")) {
                    // qs >= mentioned means the quest has been mentioned once, but may also be started
                    result.append(QString("Quests.isKnown(%1)").arg(questId));
                } else if (isIdentifier(right, "qs_accepted") || isIdentifier(right, "qs_acccepted")) {
                    // qs >= qs_accepted means the quest has been started
                    result.append(QString("Quests.isStarted(%1)").arg(questId));
                } else if (isIdentifier(right, "qs_completed")) {
                    // qs >= qs_completed means the quest has been completed
                    result.append(QString("Quests.isFinished(%1)").arg(questId));
                } else {
                    QString value;
                    convertExpression(right, value, 0, environment, false);
                    qWarning("Comparing (GTE) quest state to unknown value: %s", qPrintable(value));
                }
            } else if (op == Gt) {
                if (isIdentifier(right, "qs_unknown")) {
                    // qs > unknown means the quest has been mentioned once, but may also be started
                    result.append(QString("Quests.isKnown(%1)").arg(questId));
                } else if (isIdentifier(right, "qs_mentioned")) {
                    // qs > qs_mentioned means the quest has been started
                    result.append(QString("Quests.isStarted(%1)").arg(questId));
                } else {
                    QString value;
                    convertExpression(right, value, 0, environment, false);
                    qWarning("Comparing (GT) quest state to unknown value: %s", qPrintable(value));
                }
            } else {
                QString valueText;
                convertExpression(right, valueText, 0, environment, false);
                qWarning("Comparing quest-state with unknown op: %d against %s", op, qPrintable(valueText));
            }

            return true;
        } else if (isQuestStateField(right, &questId, environment)) {
            qWarning("Quest id on right side of comparison. Didn't account for this.");
        }

        /**
          Convert reads to game.global_flags[x] to GlobalFlags.isSet(x)
          */
        QString flagId;
        if (isGlobalFlagsField(left, &flagId, environment)) {
            QString comparingTo;
            convertExpression(right, comparingTo, indent, environment, false);

            bool invert = (op == NotEq);

            if (comparingTo == "0" || comparingTo == "false") {
                if (!invert)
                    result.append("!");
            } else if (comparingTo == "1" || comparingTo == "true") {
                if (invert)
                    result.append("!");
            } else {
                qWarning("Comparing global flag to something other than 1 or 0: %s", qPrintable(comparingTo));
            }

            result.append("GlobalFlags.isSet(").append(flagId).append(")");

            return true;
        } else if (isGlobalFlagsField(right, &flagId, environment)) {
            qWarning("GlobalFlags on the right-hand-side of a comparison: Didn't handle this case.");
        }

        /**
         Convert reads to game.areas[ID] into direct "isKnown" calls for the world map.
         */
        QString mapId;
        if (isAreasField(left, &mapId, environment)) {
            QString comparingTo;
            convertExpression(right, comparingTo, indent, environment, false);

            if (comparingTo == "0")
                result.append("!");
            else if (comparingTo != "1")
            {
                qWarning("Comparing area %s to unknown constant %s.", qPrintable(mapId), qPrintable(comparingTo));
                return false;
            }

            bool ok;
            uint realMapId = mapId.toUInt(&ok);

            if (!ok || realMapId < 1 || realMapId > 12) {
                qDebug("Unknown area id encountered: %s", qPrintable(mapId));
                result.append("WorldMap.isMarked('area-").append(mapId).append("')");
            } else {
                result.append("WorldMap.isMarked(").append(areaIds[realMapId]).append(")");
                return true;
            }
        }
    }

    return false;
}

static bool process(stmt_ty stmt, QString &result, int indent, Environment *environment)
{
    if (stmt->kind == Assign_kind) {
        if (asdl_seq_LEN(stmt->v.Assign.targets) > 1) {
            return false;
        }

        expr_ty target = (expr_ty)asdl_seq_GET(stmt->v.Assign.targets, 0);
        expr_ty value = stmt->v.Assign.value;

        QString questId, flagId, varId;
        if (isQuestStateField(target, &questId, environment)) {
            processQuestId(questId);

            appendIndent(indent, result);
            if (isIdentifier(value, "qs_mentioned")) {
                result.append(QString("Quests.mention(%1);\n").arg(questId));
            } else if (isIdentifier(value, "qs_accepted") || isIdentifier(value, "qs_acccepted")) {
                result.append(QString("Quests.accept(%1);\n").arg(questId));
            } else if (isIdentifier(value, "qs_completed")) {
                result.append(QString("Quests.complete(%1);\n").arg(questId));
            } else if (isIdentifier(value, "qs_botched")) {
                result.append(QString("Quests.botch(%1);\n").arg(questId));
            } else {
                QString valuetext;
                convertExpression(value, valuetext, 0, environment, false);
                qWarning("Setting quest state to invalid value: %s", qPrintable(valuetext));
            }
            return true;
        } else if (isGlobalFlagsField(target, &flagId, environment)) {
            QString newValue;
            convertExpression(value, newValue, indent, environment, true);

            if (newValue == "1") {
                appendIndent(indent, result);
                result.append("GlobalFlags.set(").append(flagId).append(");\n");
            } else if (newValue == "0") {
                result.append("GlobalFlags.unset(").append(flagId).append(");\n");
            } else {
                qWarning("Invalid value for global flags: %s.", qPrintable(newValue));
            }
            return true;
        } else if (isGlobalVarsField(target, &varId, environment)) {
            appendIndent(indent, result);
            result.append("GlobalVars.set(").append(varId).append(", ");
            convertExpression(value, result, indent, environment, true);
            result.append(");\n");
            return true;
        }

        QString mapId;
        if (isAreasField(target, &mapId, environment)) {
            QString settingTo;
            convertExpression(value, settingTo, indent, environment, false);

            if (settingTo == "1") {
                appendIndent(indent, result);
                result.append("WorldMap.mark(").append(mapId).append(");\n");
                return true;
            } else {
                qWarning("Setting area %s to unknown constant %s.", qPrintable(mapId), qPrintable(settingTo));
                return false;
            }
        }
    }

    return false;
}
