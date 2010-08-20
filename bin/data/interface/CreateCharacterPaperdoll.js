
function setCharacter(character) {
    // Set experience points
    if (character.experience !== undefined) {
        xpDisplay.text = character.experience;
        grayOutExp.visible = false;
    } else {
        grayOutExp.visible = true;
    }

    // Set level
    if (character.level !== undefined) {
        levelDisplay.text = character.level;
        grayOutLevel.visible = false;
    } else {
        grayOutLevel.visible = true;
    }

    // Set stats
    if (character.strength !== undefined) {
        strDisplay.text = character.strength;
        strModDisplay.text = Utilities.getAbilityModifier(character.strength);
        grayOutStr.visible = false;
    } else {
        grayOutStr.visible = true;
    }

    if (character.dexterity !== undefined) {
        dexDisplay.text = character.dexterity;
        dexModDisplay.text = Utilities.getAbilityModifier(character.dexterity);
        grayOutDex.visible = false;
    } else {
        grayOutDex.visible = true;
    }

    if (character.constitution !== undefined) {
        conDisplay.text = character.constitution;
        conModDisplay.text = Utilities.getAbilityModifier(character.constitution);
        grayOutCon.visible = false;
    } else {
        grayOutCon.visible = true;
    }

    if (character.intelligence !== undefined) {
        intDisplay.text = character.intelligence;
        intModDisplay.text = Utilities.getAbilityModifier(character.intelligence);
        grayOutInt.visible = false;
    } else {
        grayOutInt.visible = true;
    }

    if (character.wisdom !== undefined) {
        wisDisplay.text = character.wisdom;
        wisModDisplay.text = Utilities.getAbilityModifier(character.wisdom);
        grayOutWis.visible = false;
    } else {
        grayOutWis.visible = true;
    }

    if (character.charisma !== undefined) {
        chaDisplay.text = character.charisma;
        chaModDisplay.text = Utilities.getAbilityModifier(character.charisma);
        grayOutCha.visible = false;
    } else {
        grayOutCha.visible = true;
    }

    // HP
    if (character.hitPoints !== undefined) {
        hpDisplay.text = character.hitPoints;
        grayOutHp.visible = false;
    } else {
        grayOutHp.visible = true;
    }

    // AC
    if (character.armorClass !== undefined) {
        acDisplay.text = character.armorClass;
        grayOutAc.visible = false;
    } else {
        grayOutAc.visible = true;
    }

    // Saves
    if (character.fortitudeSave !== undefined) {
        fortDisplay.text = character.fortitudeSave;
        grayOutFort.visible = false;
    } else {
        grayOutFort.visible = true;
    }
    if (character.willSave !== undefined) {
        willDisplay.text = character.willSave;
        grayOutWill.visible = false;
    } else {
        grayOutWill.visible = true;
    }
    if (character.reflexSave !== undefined) {
        refDisplay.text = character.reflexSave;
        grayOutRef.visible = false;
    } else {
        grayOutRef.visible = true;
    }

    // Initiative
    if (character.initiative !== undefined) {
        initDisplay.text = character.initiative;
        grayOutInit.visible = false;
    } else {
        grayOutInit.visible = true;
    }

    // Speed
    if (character.speed !== undefined) {
        speedDisplay.text = character.speed;
        grayOutSpeed.visible = false;
    } else {
        grayOutSpeed.visible = true;
    }

    // Attack boni
    if (character.meleeBonus !== undefined) {
        meleeDisplay.text = character.meleeBonus;
        grayOutMelee.visible = false;
    } else {
        grayOutMelee.visible = true;
    }

    // Attack boni
    if (character.rangedBonus !== undefined) {
        rangedDisplay.text = character.rangedBonus;
        grayOutRanged.visible = false;
    } else {
        grayOutRanged.visible = true;
    }

    // Char height + weight
    if (character.height !== undefined) {
        heightDisplay.text = character.height;
        grayOutHeight.visible = false;
    } else {
        grayOutHeight.visible = true;
    }
    if (character.weight !== undefined) {
        weightDisplay.text = character.weight;
        grayOutWeight.visible = false;
    } else {
        grayOutWeight.visible = true;
    }

}
