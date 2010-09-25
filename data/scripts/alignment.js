/**
 * Defines the matrix of compatible party alignments.
 */
var CompatibleAlignments = {};

CompatibleAlignments[ Alignment.LawfulGood ] = [Alignment.LawfulGood, Alignment.NeutralGood, Alignment.LawfulNeutral];
CompatibleAlignments[ Alignment.NeutralGood ] = [Alignment.LawfulGood, Alignment.NeutralGood, Alignment.ChaoticGood, Alignment.TrueNeutral];
CompatibleAlignments[ Alignment.ChaoticGood ] = [Alignment.NeutralGood, Alignment.ChaoticGood, Alignment.ChaoticNeutral];
CompatibleAlignments[ Alignment.LawfulNeutral ] = [Alignment.LawfulGood, Alignment.LawfulNeutral, Alignment.TrueNeutral, Alignment.LawfulEvil];
CompatibleAlignments[ Alignment.TrueNeutral ] = [Alignment.NeutralGood, Alignment.LawfulNeutral, Alignment.TrueNeutral, Alignment.ChaoticNeutral, Alignment.NeutralEvil];
CompatibleAlignments[ Alignment.ChaoticNeutral ] = [Alignment.ChaoticGood, Alignment.TrueNeutral, Alignment.ChaoticGood, Alignment.ChaoticEvil];
CompatibleAlignments[ Alignment.LawfulEvil ] = [Alignment.LawfulNeutral, Alignment.LawfulEvil, Alignment.NeutralEvil];
CompatibleAlignments[ Alignment.NeutralEvil ] = [Alignment.TrueNeutral, Alignment.LawfulEvil, Alignment.NeutralEvil, Alignment.ChaoticEvil];
CompatibleAlignments[ Alignment.ChaoticEvil ] = [Alignment.ChaoticNeutral, Alignment.NeutralEvil, Alignment.ChaoticEvil];

/**
 * Human-readable names for the alignments.
 */
var AlignmentNames = {};

(function() {

    function setupAlignmentNames() {
        AlignmentNames[ Alignment.LawfulGood ] = translations.get('mes/stat/6005');
        AlignmentNames[ Alignment.NeutralGood ] = translations.get('mes/stat/6004');
        AlignmentNames[ Alignment.ChaoticGood ] = translations.get('mes/stat/6006');
        AlignmentNames[ Alignment.LawfulNeutral ] = translations.get('mes/stat/6001');
        AlignmentNames[ Alignment.TrueNeutral ] = translations.get('mes/stat/6000');
        AlignmentNames[ Alignment.ChaoticNeutral ] = translations.get('mes/stat/6002');
        AlignmentNames[ Alignment.LawfulEvil ] = translations.get('mes/stat/6009');
        AlignmentNames[ Alignment.NeutralEvil ] = translations.get('mes/stat/6008');
        AlignmentNames[ Alignment.ChaoticEvil ] = translations.get('mes/stat/6010');
    }

    StartupListeners.add(setupAlignmentNames, 'alignment-names');

})();
