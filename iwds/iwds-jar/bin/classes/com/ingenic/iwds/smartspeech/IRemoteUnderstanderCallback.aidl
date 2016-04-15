/*
 *  Copyright (C) 2014 Ingenic Semiconductor
 *
 *  ZhangYanMing <yanming.zhang@ingenic.com, jamincheung@126.com>
 *
 *  Elf/IDWS Project
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

package com.ingenic.iwds.smartspeech;

import com.ingenic.iwds.smartspeech.business.RemoteBusiness;

interface IRemoteUnderstanderCallback {
        void onListeningStatus(boolean isListening);
        void onBeginOfSpeech();
        void onEndOfSpeech();
        void onCancel();
        void onError(int errorCode);
        void onResult(in RemoteBusiness result);
        void onVolumeChanged(int volume);
}