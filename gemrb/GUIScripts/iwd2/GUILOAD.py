# -*-python-*-
# GemRB - Infinity Engine Emulator
# Copyright (C) 2003 The GemRB Project
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#


# GUILOAD.py - Load window

###################################################

import GemRB
import LoadScreen
from GameCheck import MAX_PARTY_SIZE
from GUIDefines import *

LoadWindow = 0
TextAreaControl = 0
Games = ()
ScrollBar = 0

def OnLoad ():
	global LoadWindow, TextAreaControl, Games, ScrollBar

	GemRB.SetToken ("SaveDir", "mpsave") # iwd2 is always using 'mpsave'
	LoadWindow = GemRB.LoadWindow (0, "GUILOAD")

	CancelButton=LoadWindow.GetControl (22)
	CancelButton.SetText (13727)
	CancelButton.SetEvent (IE_GUI_BUTTON_ON_PRESS, lambda: LoadWindow.Close())
	CancelButton.MakeEscape()

	for i in range (5):
		Button = LoadWindow.GetControl (55+i)
		Button.SetText (15590)
		Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, LoadGamePress)
		Button.SetState (IE_GUI_BUTTON_DISABLED)
		Button.SetValue (i)

		Button = LoadWindow.GetControl (60+i)
		Button.SetText (13957)
		Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, DeleteGamePress)
		Button.SetState (IE_GUI_BUTTON_DISABLED)
		Button.SetValue (i)

		#area previews
		Button = LoadWindow.GetControl (1+i)
		Button.SetState (IE_GUI_BUTTON_LOCKED)
		Button.SetFlags (IE_GUI_BUTTON_NO_IMAGE|IE_GUI_BUTTON_PICTURE,OP_SET)

		#PC portraits
		for j in range (min(6, MAX_PARTY_SIZE)):
			Button = LoadWindow.GetControl (25 + i*min(6, MAX_PARTY_SIZE) + j)
			Button.SetState (IE_GUI_BUTTON_LOCKED)
			Button.SetFlags (IE_GUI_BUTTON_NO_IMAGE|IE_GUI_BUTTON_PICTURE,OP_SET)
			Button.SetSize (21, 21)

	ScrollBar=LoadWindow.GetControl (23)
	ScrollBar.SetEvent (IE_GUI_SCROLLBAR_ON_CHANGE, ScrollBarPress)
	Games=GemRB.GetSaveGames()
	TopIndex = max (0, len(Games) - 5)

	ScrollBar.SetVarAssoc ("TopIndex", TopIndex, 0, TopIndex)
	ScrollBarPress ()

	LoadWindow.SetEventProxy(ScrollBar)
	LoadWindow.Focus()
	return

def ScrollBarPress ():
	#draw load game portraits
	Pos = GemRB.GetVar ("TopIndex")
	for i in range (5):
		ActPos = Pos + i

		Button1 = LoadWindow.GetControl (55+i)
		Button2 = LoadWindow.GetControl (60+i)
		if ActPos<len(Games):
			Button1.SetState (IE_GUI_BUTTON_ENABLED)
			Button2.SetState (IE_GUI_BUTTON_ENABLED)
		else:
			Button1.SetState (IE_GUI_BUTTON_DISABLED)
			Button2.SetState (IE_GUI_BUTTON_DISABLED)

		if ActPos<len(Games):
			Slotname = Games[ActPos].GetName()
		else:
			Slotname = ""
		Label = LoadWindow.GetControl (0x10000005+i)
		Label.SetText (Slotname)

		if ActPos<len(Games):
			Slotname = Games[ActPos].GetGameDate()
		else:
			Slotname = ""
		Label = LoadWindow.GetControl (0x1000000a+i)
		Label.SetText (Slotname)

		if ActPos<len(Games):
			Slotname = Games[ActPos].GetDate()
		else:
			Slotname = ""
		Label = LoadWindow.GetControl (0x1000000f+i)
		Label.SetText (Slotname)

		Button=LoadWindow.GetControl (1+i)
		if ActPos<len(Games):
			Button.SetSprite2D(Games[ActPos].GetPreview())
		else:
			Button.SetPicture (None)
		for j in range (min(6, MAX_PARTY_SIZE)):
			Button=LoadWindow.GetControl (25 + i*min(6, MAX_PARTY_SIZE) + j)
			if ActPos<len(Games):
				Button.SetSprite2D(Games[ActPos].GetPortrait(j))
			else:
				Button.SetPicture (None)
	return

def LoadGamePress (btn):
	if LoadWindow:
		LoadWindow.Unload ()
	Pos = GemRB.GetVar ("TopIndex") + btn.Value
	LoadScreen.StartLoadScreen()
	GemRB.LoadGame(Games[Pos]) #loads and enters savegame
	GemRB.EnterGame ()
	return

def DeleteGameConfirm (btn):
	global Games

	TopIndex = GemRB.GetVar ("TopIndex")
	Pos = TopIndex + btn.Value
	GemRB.DeleteSaveGame(Games[Pos])
	del Games[Pos]
	if TopIndex > 0:
		TopIndex = TopIndex - 1
	ScrollBar.SetVarAssoc ("TopIndex", TopIndex, 0, max (0, len(Games) - 5))
	ScrollBarPress ()
	if ConfirmWindow:
		ConfirmWindow.Unload ()
	LoadWindow.Focus()
	return

def DeleteGameCancel ():
	if ConfirmWindow:
		ConfirmWindow.Unload ()
	LoadWindow.Focus()
	return

def DeleteGamePress (btn):
	global ConfirmWindow

	ConfirmWindow=GemRB.LoadWindow (1)
	ConfirmWindow.SetFlags (WF_ALPHA_CHANNEL, OP_OR)

	Text=ConfirmWindow.GetControl (0)
	Text.SetText (15305)
	DeleteButton=ConfirmWindow.GetControl (1)
	DeleteButton.SetText (13957)
	DeleteButton.SetEvent (IE_GUI_BUTTON_ON_PRESS, DeleteGameConfirm)
	DeleteButton.SetValue (btn.Value)
	CancelButton=ConfirmWindow.GetControl (2)
	CancelButton.SetText (13727)
	CancelButton.SetEvent (IE_GUI_BUTTON_ON_PRESS, DeleteGameCancel)
	CancelButton.MakeEscape()

	ConfirmWindow.ShowModal (MODAL_SHADOW_GRAY)
	return
