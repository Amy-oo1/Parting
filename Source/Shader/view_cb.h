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

namespace Shader {
	using PlanarViewConstants = ::PlanarViewConstants;
}

#endif // VIEW_CB_H