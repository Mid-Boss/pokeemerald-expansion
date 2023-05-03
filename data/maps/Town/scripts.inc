Town_MapScripts::
	.byte 0

Town_NPCGiveGen1Mon::
	lock
	faceplayer
	messageinstant Town_Text_Choice
	getrandommon 1, 0
	bufferspeciesname STR_VAR_1, VAR_RESULT
	copyvar VAR_TEMP_1, VAR_RESULT
	dynmultipush Town_Text_Choice_1, VAR_RESULT
	getrandommon 1, 0
	bufferspeciesname STR_VAR_2, VAR_RESULT
	copyvar VAR_TEMP_2, VAR_RESULT
	dynmultipush Town_Text_Choice_2, VAR_RESULT
	dynmultistack 0, 0, TRUE, 6, FALSE, 0, DYN_MULTICHOICE_CB_SHOW_MON 
	bufferspeciesname STR_VAR_3, VAR_RESULT
	givemon VAR_RESULT, 50
	msgbox Town_Text_NPCGiveGen1Mon, MSGBOX_NPC
	release
	end

Town_NPCGiveGen2Mon::
	getrandommon 2, 0
	bufferspeciesname 0, VAR_RESULT
	msgbox Town_Text_NPCGiveGen2Mon, MSGBOX_NPC
	end

Town_NPCGiveGen3Mon::
	getrandommon 3, 0
	bufferspeciesname 0, VAR_RESULT
	msgbox Town_Text_NPCGiveGen3Mon, MSGBOX_NPC
	end

Town_NPCGiveGen4Mon::
	getrandommon 4, 0
	bufferspeciesname 0, VAR_RESULT
	msgbox Town_Text_NPCGiveGen4Mon, MSGBOX_NPC
	end

Town_NPCGiveGen5Mon::
	getrandommon 5, 0
	bufferspeciesname 0, VAR_RESULT
	msgbox Town_Text_NPCGiveGen5Mon, MSGBOX_NPC
	end

Town_NPCGiveGen6Mon::
	getrandommon 6, 0
	bufferspeciesname 0, VAR_RESULT
	msgbox Town_Text_NPCGiveGen6Mon, MSGBOX_NPC
	end

Town_NPCGiveGen7Mon::
	getrandommon 7, 0
	bufferspeciesname 0, VAR_RESULT
	msgbox Town_Text_NPCGiveGen7Mon, MSGBOX_NPC
	end

Town_NPCGiveGen8Mon::
	getrandommon 8, 0
	bufferspeciesname 0, VAR_RESULT
	msgbox Town_Text_NPCGiveGen8Mon, MSGBOX_NPC
	end

Town_NPCGiveGenAnyMon::
	getrandommon 0, 0
	bufferspeciesname 0, VAR_RESULT
	msgbox Town_Text_NPCGiveGenAnyMon, MSGBOX_NPC
	end

Town_GiveMon_1::
	givemon VAR_TEMP_1, 50
	msgbox Town_Text_NPCGiveGen1Mon, MSGBOX_NPC
	release
	end

Town_GiveMon_2::
	givemon VAR_TEMP_2, 50
	msgbox Town_Text_NPCGiveGen1Mon, MSGBOX_NPC
	release
	end

Town_Text_Choice:
	.string "Choose a Pokemon to take with you.$"

Town_Text_Choice_1:
	.string "{STR_VAR_1}$"
Town_Text_Choice_2:
	.string "{STR_VAR_2}$"

Town_Text_NPCGiveGen1Mon:
	.string "Here is your Gen 1 pokemon!\n"
	.string "{STR_VAR_3}$"
Town_Text_NPCGiveGen2Mon:
	.string "Here is your Gen 2 pokemon!\n"
	.string "{STR_VAR_3}$"
Town_Text_NPCGiveGen3Mon:
	.string "Here is your Gen 3 pokemon!\n"
	.string "{STR_VAR_3}$"
Town_Text_NPCGiveGen4Mon:
	.string "Here is your Gen 4 pokemon!\n"
	.string "{STR_VAR_3}$"
Town_Text_NPCGiveGen5Mon:
	.string "Here is your Gen 5 pokemon!\n"
	.string "{STR_VAR_3}$"
Town_Text_NPCGiveGen6Mon:
	.string "Here is your Gen 6 pokemon!\n"
	.string "{STR_VAR_3}$"
Town_Text_NPCGiveGen7Mon:
	.string "Here is your Gen 7 pokemon!\n"
	.string "{STR_VAR_3}$"
Town_Text_NPCGiveGen8Mon:
	.string "Here is your Gen 8 pokemon!\n"
	.string "{STR_VAR_3}$"
Town_Text_NPCGiveGenAnyMon:
	.string "Here is your random pokemon!\n"
	.string "{STR_VAR_1}$"
