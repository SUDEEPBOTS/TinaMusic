/*
 * ============================================================================
 *   ████████╗██╗███╗   ██╗ █████╗ ███╗   ███╗██╗   ██╗███████╗██╗ ██████╗
 *   ╚══██╔══╝██║████╗  ██║██╔══██╗████╗ ████║██║   ██║██╔════╝██║██╔════╝
 *      ██║   ██║██╔██╗ ██║███████║██╔████╔██║██║   ██║███████╗██║██║     
 *      ██║   ██║██║╚██╗██║██╔══██║██║╚██╔╝██║██║   ██║╚════██║██║██║     
 *      ██║   ██║██║ ╚████║██║  ██║██║ ╚═╝ ██║╚██████╔╝███████║██║╚██████╗
 *      ╚═╝   ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝     ╚═╝ ╚═════╝ ╚══════╝╚═╝ ╚═════╝
 * ============================================================================
 *
 * Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
 * Architecture: Native C++20 Core + Go MTProto WebRTC Voice Bridge
 * Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 * Support: https://t.me/Z0iiw
 *
 * ----------------------------------------------------------------------------
 * LEGAL NOTICE & LICENSE (GNU Affero General Public License v3.0):
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * STRICT COPYLEFT CLAUSE:
 * In accordance with Section 13 of the GNU AGPLv3, if you modify this program
 * or run it as a network service (e.g., hosted on servers, Telegram, or cloud),
 * you MUST make the complete source code of your modified version available
 * to all users under the exact same AGPLv3 license terms.
 * Closed-source derivatives or rebranded copies are strictly prohibited.
 * ============================================================================
 */

package ntgcalls

//#include "ntgcalls.h"
import "C"

type (
	StreamType          int
	ConnectionMode      int
	ConnectionKind      int
	ConnectionState     int
	StreamStatus        int
	StreamMode          int
	StreamDevice        int
	MediaSource         int
	MediaSegmentQuality int
	MediaSegmentStatus  int
)

type (
	StreamEndCallback          func(chatId int64, streamType StreamType, streamDevice StreamDevice)
	UpgradeCallback            func(chatId int64, state MediaState)
	ConnectionChangeCallback   func(chatId int64, state NetworkInfo)
	SignalCallback             func(chatId int64, signal []byte)
	FrameCallback              func(chatId int64, mode StreamMode, device StreamDevice, frames []Frame)
	RemoteSourceCallback       func(chatId int64, source RemoteSource)
	BroadcastTimestampCallback func(chatId int64)
)

type BroadcastPartCallback func(chatId int64, segmentPartRequest SegmentPartRequest)

const (
	MicrophoneStream StreamDevice = iota
	SpeakerStream
	CameraStream
	ScreenStream
)

const (
	AudioStream StreamType = iota
	VideoStream
)

const (
	MediaSourceFile MediaSource = 1 << iota
	MediaSourceShell
	MediaSourceFFmpeg
	MediaSourceDevice
	MediaSourceDesktop
	MediaSourceExternal
)

const (
	ActiveStream StreamStatus = iota
	PausedStream
	IdlingStream
)

const (
	RtcConnection ConnectionMode = iota
	StreamConnection
	RTMPConnection
)

const (
	Connecting ConnectionState = iota
	Connected
	Failed
	Timeout
	Closed
)

const (
	NormalConnection ConnectionKind = iota
	PresentationConnection
)

const (
	CaptureStream StreamMode = iota
	PlaybackStream
)

const (
	SegmentQualityNone MediaSegmentQuality = iota - 1
	SegmentQualityThumbnail
	SegmentQualityMedium
	SegmentQualityFull
)

const (
	SegmentStatusNotReady MediaSegmentStatus = iota
	SegmentStatusResyncNeeded
	SegmentStatusSuccess
)

func (ctx MediaSource) ParseToC() C.ntg_media_source_enum {
	switch ctx {
	case MediaSourceFile:
		return C.NTG_FILE
	case MediaSourceShell:
		return C.NTG_SHELL
	case MediaSourceFFmpeg:
		return C.NTG_FFMPEG
	case MediaSourceDevice:
		return C.NTG_DEVICE
	case MediaSourceDesktop:
		return C.NTG_DESKTOP
	case MediaSourceExternal:
		return C.NTG_EXTERNAL
	default:
		return C.NTG_FILE
	}
}

func (ctx StreamMode) ParseToC() C.ntg_stream_mode_enum {
	switch ctx {
	case CaptureStream:
		return C.NTG_STREAM_CAPTURE
	case PlaybackStream:
		return C.NTG_STREAM_PLAYBACK
	default:
		return C.NTG_STREAM_CAPTURE
	}
}

func (ctx StreamDevice) ParseToC() C.ntg_stream_device_enum {
	switch ctx {
	case MicrophoneStream:
		return C.NTG_STREAM_MICROPHONE
	case SpeakerStream:
		return C.NTG_STREAM_SPEAKER
	case CameraStream:
		return C.NTG_STREAM_CAMERA
	case ScreenStream:
		return C.NTG_STREAM_SCREEN
	default:
		return C.NTG_STREAM_MICROPHONE
	}
}

func (ctx MediaSegmentStatus) ParseToC() C.ntg_media_segment_status_enum {
	switch ctx {
	case SegmentStatusNotReady:
		return C.NTG_MEDIA_SEGMENT_NOT_READY
	case SegmentStatusResyncNeeded:
		return C.NTG_MEDIA_SEGMENT_RESYNC_NEEDED
	case SegmentStatusSuccess:
		return C.NTG_MEDIA_SEGMENT_SUCCESS
	default:
		return C.NTG_MEDIA_SEGMENT_NOT_READY
	}
}
