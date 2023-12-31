// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PostProcess/PostProcessEyeAdaptation.h"
#include "PostProcess/PostProcessBloomSetup.h"
#include "ScreenPass.h"
#include "OverridePassSequence.h"
#include "Math/Halton.h"


bool SupportsFilmGrain(EShaderPlatform Platform);

BEGIN_SHADER_PARAMETER_STRUCT(FTonemapperOutputDeviceParameters, )
	SHADER_PARAMETER(FVector3f, InverseGamma)
	SHADER_PARAMETER(uint32, OutputDevice)
	SHADER_PARAMETER(uint32, OutputGamut)
	SHADER_PARAMETER(float, OutputMaxLuminance)
	END_SHADER_PARAMETER_STRUCT()

RENDERER_API FTonemapperOutputDeviceParameters GetTonemapperOutputDeviceParameters(const FSceneViewFamily& Family);

static void GrainRandomFromFrame(FVector3f* RESTRICT const Constant, uint32 FrameNumber)
{
	Constant->X = Halton(FrameNumber & 1023, 2);
	Constant->Y = Halton(FrameNumber & 1023, 3);
}

struct FTonemapInputs
{
	// [Optional] Render to the specified output. If invalid, a new texture is created and returned.
	FScreenPassRenderTarget OverrideOutput;

	// [Required] HDR scene color to tonemap.
	FScreenPassTexture SceneColor;

	// [Required] Filtered bloom texture to composite with tonemapped scene color. This should be transparent black for no bloom.
	FScreenPassTexture Bloom;

	// [Optional] structured buffer of multiply parameters to apply to the scene color.
	FRDGBufferRef SceneColorApplyParamaters = nullptr;

	// [Optional] Luminance bilateral grid. If this is null, local exposure is disabled.
	FRDGTextureRef LocalExposureTexture = nullptr;

	// [Optional] Blurred luminance texture used to calculate local exposure.
	FRDGTextureRef BlurredLogLuminanceTexture = nullptr;

	// [Required] Eye adaptation parameters.
	const FEyeAdaptationParameters* EyeAdaptationParameters = nullptr;

	// [Required] Color grading texture used to remap colors.
	FRDGTextureRef ColorGradingTexture = nullptr;

	// [Optional, SM5+] Eye adaptation texture used to compute exposure. If this is null, a default exposure value is used instead.
	FRDGTextureRef EyeAdaptationTexture = nullptr;

	// [Optional, ES31] Eye adaptation buffer used to compute exposure. 
	FRDGBufferRef EyeAdaptationBuffer = nullptr;

	// [Raster Only] Controls whether the alpha channel of the scene texture should be written to the output texture.
	bool bWriteAlphaChannel = false;

	// Configures the tonemapper to only perform gamma correction.
	bool bGammaOnly = false;

	// Whether to leave the final output in HDR.
	bool bOutputInHDR = false;

	bool bMetalMSAAHDRDecode = false;

	// Mobile tonemap subpass 
	bool bUseSubpass = false;

	// [Optional] Mobile tonemap subpass number of MSAA samples read from scene color. Subpass pixel shader peforms resolve so will always output a single sample. 
	uint32 MsaaSamples = 1;

	// [Optional] Mobile tonemap subpass target size.
	FIntPoint TargetSize;

	// Returns whether ApplyParameters is supported by the tonemapper.
	static bool SupportsSceneColorApplyParametersBuffer(EShaderPlatform Platform);
};

FScreenPassTexture AddTonemapPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, const FTonemapInputs& Inputs);
