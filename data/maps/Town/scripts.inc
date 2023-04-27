Town_MapScripts::
	.byte 0

Town_NPCGiveMon::
	giverandommon 0
	bufferspeciesname 0, VAR_RESULT
	msgbox Town_Text_NPCGiveMon, MSGBOX_NPC
	end

Town_Text_NPCGiveMon:
	.string "Here is a {STR_VAR_1}$"
