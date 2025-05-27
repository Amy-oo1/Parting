/*
* Copyright (c) 2014-2021, NVIDIA CORPORATION. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#ifndef VIEW_CB_H
#define VIEW_CB_H

struct PlanarViewConstants {
	Math::MatF44		MatWorldToView;
	Math::MatF44		MatViewToClip;
	Math::MatF44		MatWorldToClip;
	Math::MatF44		MatClipToView;
	Math::MatF44		MatViewToWorld;
	Math::MatF44		MatClipToWorld;

	Math::MatF44		MatViewToClipNoOffset;
	Math::MatF44		MatWorldToClipNoOffset;
	Math::MatF44		MatClipToViewNoOffset;
	Math::MatF44		MatClipToWorldNoOffset;

	Math::VecF2			ViewportOrigin;
	Math::VecF2			ViewportSize;

	Math::VecF2			ViewportSizeInv;
	Math::VecF2			PixelOffset;

	Math::VecF2			ClipToWindowScale;
	Math::VecF2			ClipToWindowBias;

	Math::VecF2			WindowToClipScale;
	Math::VecF2			WindowToClipBias;

	Math::VecF4			CameraDirectionOrPosition;
};

#endif // VIEW_CB_H