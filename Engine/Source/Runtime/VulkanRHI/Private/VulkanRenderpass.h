// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	VulkanState.h: Vulkan state definitions.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "RHI.h"
#include "VulkanRHIPrivate.h"
#include "VulkanResources.h"
#include "VulkanPendingState.h"
#include "VulkanContext.h"

template <typename TAttachmentReferenceType>
struct FVulkanAttachmentReference
	: public TAttachmentReferenceType
{
	FVulkanAttachmentReference()
	{
		ZeroStruct();
	}

	FVulkanAttachmentReference(const VkAttachmentReference& AttachmentReferenceIn, VkImageAspectFlags AspectMask)
	{
		SetAttachment(AttachmentReferenceIn, AspectMask);
	}

	inline void SetAttachment(const VkAttachmentReference& AttachmentReferenceIn, VkImageAspectFlags AspectMask) { check(false); }
	inline void SetAttachment(const FVulkanAttachmentReference<TAttachmentReferenceType>& AttachmentReferenceIn, VkImageAspectFlags AspectMask) { *this = AttachmentReferenceIn; }
	inline void ZeroStruct() {}
	inline void SetAspect(uint32 Aspect) {}
};

template <>
inline void FVulkanAttachmentReference<VkAttachmentReference>::SetAttachment(const VkAttachmentReference& AttachmentReferenceIn, VkImageAspectFlags AspectMask)
{
	attachment = AttachmentReferenceIn.attachment;
	layout = AttachmentReferenceIn.layout;
}

#if VULKAN_SUPPORTS_RENDERPASS2
template <>
inline void FVulkanAttachmentReference<VkAttachmentReference2>::SetAttachment(const VkAttachmentReference& AttachmentReferenceIn, VkImageAspectFlags AspectMask)
{
	sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
	pNext = nullptr;
	attachment = AttachmentReferenceIn.attachment;
	layout = AttachmentReferenceIn.layout;
	aspectMask = AspectMask;
}

template<>
inline void FVulkanAttachmentReference<VkAttachmentReference2>::SetAttachment(const FVulkanAttachmentReference<VkAttachmentReference2>& AttachmentReferenceIn, VkImageAspectFlags AspectMask)
{
	sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
	pNext = nullptr;
	attachment = AttachmentReferenceIn.attachment;
	layout = AttachmentReferenceIn.layout;
	aspectMask = AspectMask;
}
#endif

template<>
inline void FVulkanAttachmentReference<VkAttachmentReference>::ZeroStruct()
{
	attachment = 0;
	layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

#if VULKAN_SUPPORTS_RENDERPASS2
template<>
inline void FVulkanAttachmentReference<VkAttachmentReference2>::ZeroStruct()
{
	sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
	pNext = nullptr;
	attachment = 0;
	layout = VK_IMAGE_LAYOUT_UNDEFINED;
	aspectMask = 0;
}

template<>
inline void FVulkanAttachmentReference<VkAttachmentReference2>::SetAspect(uint32 Aspect)
{
	aspectMask = Aspect;
}
#endif

template <typename TSubpassDescriptionType>
class FVulkanSubpassDescription
{
};

template<>
struct FVulkanSubpassDescription<VkSubpassDescription>
	: public VkSubpassDescription
{
	FVulkanSubpassDescription()
	{
		FMemory::Memzero(this, sizeof(VkSubpassDescription));
		pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	}

	void SetColorAttachments(const TArray<FVulkanAttachmentReference<VkAttachmentReference>>& ColorAttachmentReferences, int OverrideCount = -1)
	{
		colorAttachmentCount = (OverrideCount == -1) ? ColorAttachmentReferences.Num() : OverrideCount;
		pColorAttachments = ColorAttachmentReferences.GetData();
	}

	void SetResolveAttachments(const TArray<FVulkanAttachmentReference<VkAttachmentReference>>& ResolveAttachmentReferences)
	{
		if (ResolveAttachmentReferences.Num() > 0)
		{
			check(colorAttachmentCount == ResolveAttachmentReferences.Num());
			pResolveAttachments = ResolveAttachmentReferences.GetData();
		}
	}

	void SetDepthStencilAttachment(FVulkanAttachmentReference<VkAttachmentReference>* DepthStencilAttachmentReference)
	{
		pDepthStencilAttachment = static_cast<VkAttachmentReference*>(DepthStencilAttachmentReference);
	}

	void SetInputAttachments(FVulkanAttachmentReference<VkAttachmentReference>* InputAttachmentReferences, uint32 NumInputAttachmentReferences)
	{
		pInputAttachments = static_cast<VkAttachmentReference*>(InputAttachmentReferences);
		inputAttachmentCount = NumInputAttachmentReferences;
	}

	void SetDepthStencilResolveAttachment(VkSubpassDescriptionDepthStencilResolveKHR* DepthStencilResolveAttachmentDesc)
	{
		// No-op without VK_KHR_create_renderpass2
	}

	void SetShadingRateAttachment(VkFragmentShadingRateAttachmentInfoKHR* /* ShadingRateAttachmentInfo */)
	{
		// No-op without VK_KHR_create_renderpass2
	}

	void SetMultiViewMask(uint32_t Mask)
	{
		// No-op without VK_KHR_create_renderpass2
	}
};

#if VULKAN_SUPPORTS_RENDERPASS2
template<>
struct FVulkanSubpassDescription<VkSubpassDescription2>
	: public VkSubpassDescription2
{
	FVulkanSubpassDescription()
	{
		ZeroVulkanStruct(*this, VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2);
		pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		viewMask = 0;
	}

	void SetColorAttachments(const TArray<FVulkanAttachmentReference<VkAttachmentReference2>>& ColorAttachmentReferences, int OverrideCount = -1)
	{
		colorAttachmentCount = OverrideCount == -1 ? ColorAttachmentReferences.Num() : OverrideCount;
		pColorAttachments = ColorAttachmentReferences.GetData();
	}

	void SetResolveAttachments(const TArray<FVulkanAttachmentReference<VkAttachmentReference2>>& ResolveAttachmentReferences)
	{
		if (ResolveAttachmentReferences.Num() > 0)
		{
			check(colorAttachmentCount == ResolveAttachmentReferences.Num());
			pResolveAttachments = ResolveAttachmentReferences.GetData();
		}
	}

	void SetDepthStencilAttachment(FVulkanAttachmentReference<VkAttachmentReference2>* DepthStencilAttachmentReference)
	{
		pDepthStencilAttachment = static_cast<VkAttachmentReference2*>(DepthStencilAttachmentReference);
	}

	void SetInputAttachments(FVulkanAttachmentReference<VkAttachmentReference2>* InputAttachmentReferences, uint32 NumInputAttachmentReferences)
	{
		pInputAttachments = static_cast<VkAttachmentReference2*>(InputAttachmentReferences);
		inputAttachmentCount = NumInputAttachmentReferences;
	}

	void SetDepthStencilResolveAttachment(VkSubpassDescriptionDepthStencilResolveKHR* DepthStencilResolveAttachmentDesc)
	{
		const void* Next = pNext;
		pNext = DepthStencilResolveAttachmentDesc;
		DepthStencilResolveAttachmentDesc->pNext = Next;
	}

	void SetShadingRateAttachment(VkFragmentShadingRateAttachmentInfoKHR* ShadingRateAttachmentInfo)
	{
		const void* Next = pNext;
		pNext = ShadingRateAttachmentInfo;
		ShadingRateAttachmentInfo->pNext = Next;
	}

	void SetMultiViewMask(uint32_t Mask)
	{
		viewMask = Mask;
	}
};
#endif

template <typename TSubpassDependencyType>
struct FVulkanSubpassDependency
	: public TSubpassDependencyType
{
};

template<>
struct FVulkanSubpassDependency<VkSubpassDependency>
	: public VkSubpassDependency
{
	FVulkanSubpassDependency()
	{
		FMemory::Memzero(this, sizeof(VkSubpassDependency));
	}
};

#if VULKAN_SUPPORTS_RENDERPASS2
template<>
struct FVulkanSubpassDependency<VkSubpassDependency2>
	: public VkSubpassDependency2
{
	FVulkanSubpassDependency()
	{
		ZeroVulkanStruct(*this, VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2);
		viewOffset = 0;		// According to the Vulkan spec: "If dependencyFlags does not include VK_DEPENDENCY_VIEW_LOCAL_BIT, viewOffset must be 0"
	}
};
#endif

template<typename TAttachmentDescriptionType>
struct FVulkanAttachmentDescription
{
};

template<>
struct FVulkanAttachmentDescription<VkAttachmentDescription>
	: public VkAttachmentDescription
{
	FVulkanAttachmentDescription()
	{
		FMemory::Memzero(this, sizeof(VkAttachmentDescription));
	}

	FVulkanAttachmentDescription(const VkAttachmentDescription& InDesc)
	{
		flags = InDesc.flags;
		format = InDesc.format;
		samples = InDesc.samples;
		loadOp = InDesc.loadOp;
		storeOp = InDesc.storeOp;
		stencilLoadOp = InDesc.stencilLoadOp;
		stencilStoreOp = InDesc.stencilStoreOp;
		initialLayout = InDesc.initialLayout;
		finalLayout = InDesc.finalLayout;
	}
};

#if VULKAN_SUPPORTS_RENDERPASS2
template<>
struct FVulkanAttachmentDescription<VkAttachmentDescription2>
	: public VkAttachmentDescription2
{
	FVulkanAttachmentDescription()
	{
		ZeroVulkanStruct(*this, VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2);
	}

	FVulkanAttachmentDescription(const VkAttachmentDescription& InDesc)
	{
		sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
		pNext = nullptr;
		flags = InDesc.flags;
		format = InDesc.format;
		samples = InDesc.samples;
		loadOp = InDesc.loadOp;
		storeOp = InDesc.storeOp;
		stencilLoadOp = InDesc.stencilLoadOp;
		stencilStoreOp = InDesc.stencilStoreOp;
		initialLayout = InDesc.initialLayout;
		finalLayout = InDesc.finalLayout;
	}
};
#endif

template <typename T>
struct FVulkanRenderPassCreateInfo
{};

template<>
struct FVulkanRenderPassCreateInfo<VkRenderPassCreateInfo>
	: public VkRenderPassCreateInfo
{
	FVulkanRenderPassCreateInfo()
	{
		ZeroVulkanStruct(*this, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
	}

	void SetCorrelationMask(const uint32_t* MaskPtr)
	{
		// No-op without VK_KHR_create_renderpass2
	}

	VkRenderPass Create(FVulkanDevice& Device)
	{
		VkRenderPass Handle = VK_NULL_HANDLE;
		VERIFYVULKANRESULT_EXPANDED(VulkanRHI::vkCreateRenderPass(Device.GetInstanceHandle(), this, VULKAN_CPU_ALLOCATOR, &Handle));
		return Handle;
	}
};

struct FVulkanRenderPassFragmentDensityMapCreateInfoEXT
	: public VkRenderPassFragmentDensityMapCreateInfoEXT
{
	FVulkanRenderPassFragmentDensityMapCreateInfoEXT()
	{
		ZeroVulkanStruct(*this, VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT);
	}
};


#if VULKAN_SUPPORTS_RENDERPASS2
template<>
struct FVulkanRenderPassCreateInfo<VkRenderPassCreateInfo2>
	: public VkRenderPassCreateInfo2
{
	FVulkanRenderPassCreateInfo()
	{
		ZeroVulkanStruct(*this, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2);
	}

	void SetCorrelationMask(const uint32_t* MaskPtr)
	{
		correlatedViewMaskCount = 1;
		pCorrelatedViewMasks = MaskPtr;
	}

	VkRenderPass Create(FVulkanDevice& Device)
	{
		VkRenderPass Handle = VK_NULL_HANDLE;
		VERIFYVULKANRESULT_EXPANDED(VulkanRHI::vkCreateRenderPass2KHR(Device.GetInstanceHandle(), this, VULKAN_CPU_ALLOCATOR, &Handle));
		return Handle;
	}
};

#if VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE
struct FVulkanFragmentShadingRateAttachmentInfo
	: public VkFragmentShadingRateAttachmentInfoKHR
{
	FVulkanFragmentShadingRateAttachmentInfo()
	{
		ZeroVulkanStruct(*this, VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR);
		// For now, just use the smallest tile-size available. TODO: Add a setting to allow prioritizing either higher resolution/larger shading rate attachment targets 
		// or lower-resolution/smaller attachments.
		shadingRateAttachmentTexelSize = { (uint32)GRHIVariableRateShadingImageTileMinWidth, (uint32)GRHIVariableRateShadingImageTileMinHeight };
	}

	void SetReference(FVulkanAttachmentReference<VkAttachmentReference2>* AttachmentReference)
	{
		pFragmentShadingRateAttachment = AttachmentReference;
	}
};
#endif // VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE

#if VULKAN_SUPPORTS_DEPTH_STENCIL_RESOLVE
struct FVulkanDepthStencilResolveSubpassDesc
	: public VkSubpassDescriptionDepthStencilResolveKHR
{
	FVulkanDepthStencilResolveSubpassDesc()
	{
		ZeroVulkanStruct(*this, VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE);
	}

	void SetResolveModes(VkResolveModeFlagBits DepthMode, VkResolveModeFlagBits StencilMode)
	{
		depthResolveMode = DepthMode;
		stencilResolveMode = StencilMode;
	}

	void SetReference(FVulkanAttachmentReference<VkAttachmentReference2>* AttachmentReference)
	{
		pDepthStencilResolveAttachment = AttachmentReference;
	}
};
#endif // VULKAN_SUPPORTS_DEPTH_STENCIL_RESOLVE
#endif // VULKAN_SUPPORTS_RENDERPASS2

extern int32 GVulkanInputAttachmentShaderRead;

template <typename TSubpassDescriptionClass, typename TSubpassDependencyClass, typename TAttachmentReferenceClass, typename TAttachmentDescriptionClass, typename TRenderPassCreateInfoClass>
class FVulkanRenderPassBuilder
{
public:
	FVulkanRenderPassBuilder(FVulkanDevice& InDevice)
		: Device(InDevice)
		, CorrelationMask(0)
	{}

	void BuildCreateInfo(const FVulkanRenderTargetLayout& RTLayout)
	{
		uint32 NumSubpasses = 0;
		uint32 NumDependencies = 0;

		uint32_t SrcSubpass = 0;
		uint32_t DstSubpass = 1;

		//0b11 for 2, 0b1111 for 4, and so on
		uint32 MultiviewMask = (0b1 << RTLayout.GetMultiViewCount()) - 1;

		const bool bDeferredShadingSubpass = (RTLayout.GetSubpassHint() & ESubpassHint::DeferredShadingSubpass) != ESubpassHint::None;
		const bool bDepthReadSubpass = (RTLayout.GetSubpassHint() & ESubpassHint::DepthReadSubpass) != ESubpassHint::None;
		const bool bMobileTonemapSubpass = (RTLayout.GetSubpassHint() & ESubpassHint::MobileTonemapSubpass) != ESubpassHint::None;
#if VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE
		const bool bApplyFragmentShadingRate =
			Device.GetOptionalExtensions().HasKHRFragmentShadingRate &&
			GRHISupportsAttachmentVariableRateShading &&
			GRHIVariableRateShadingEnabled &&
			GRHIAttachmentVariableRateShadingEnabled &&
			GRHIVariableRateShadingImageDataType == VRSImage_Fractional &&
			RTLayout.GetHasFragmentDensityAttachment();
#endif // VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE

#if VULKAN_SUPPORTS_DEPTH_STENCIL_RESOLVE
		const bool bResolveDepth =
			GRHISupportsDepthStencilResolve &&
			Device.GetOptionalExtensions().HasKHRDepthStencilResolve &&
			RTLayout.GetHasDepthStencilResolve();
#endif // VULKAN_SUPPORTS_DEPTH_STENCIL_RESOLVE

#if VULKAN_SUPPORTS_RENDERPASS2
#if VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE
		if (bApplyFragmentShadingRate)
		{
			ShadingRateAttachmentReference.SetAttachment(*RTLayout.GetFragmentDensityAttachmentReference(), VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
			FragmentShadingRateAttachmentInfo.SetReference(&ShadingRateAttachmentReference);
		}
#endif // VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE

#if VULKAN_SUPPORTS_DEPTH_STENCIL_RESOLVE
		if (bResolveDepth)
		{
			DepthStencilResolveAttachmentReference.SetAttachment(*RTLayout.GetDepthStencilResolveAttachmentReference(), VkImageAspectFlagBits::VK_IMAGE_ASPECT_NONE);
			// Using zero bit because it is always supported if the extension is supported, from spec: "The VK_RESOLVE_MODE_SAMPLE_ZERO_BIT mode
			// is the only mode that is required of all implementations (that support the extension or support Vulkan 1.2 or higher)."
			DepthStencilResolveSubpassDesc.SetResolveModes(VK_RESOLVE_MODE_SAMPLE_ZERO_BIT, VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);
			DepthStencilResolveSubpassDesc.SetReference(&DepthStencilResolveAttachmentReference);
		}
#endif // VULKAN_SUPPORTS_DEPTH_STENCIL_RESOLVE
#endif // VULKAN_SUPPORTS_RENDERPASS2

		// Grab (and optionally convert) attachment references.
		uint32 NumColorAttachments = RTLayout.GetNumColorAttachments();
		for (uint32 ColorAttachment = 0; ColorAttachment < NumColorAttachments; ++ColorAttachment)
		{
			ColorAttachmentReferences.Add(TAttachmentReferenceClass(RTLayout.GetColorAttachmentReferences()[ColorAttachment], 0));
			if (RTLayout.GetResolveAttachmentReferences() != nullptr)
			{
				ResolveAttachmentReferences.Add(TAttachmentReferenceClass(RTLayout.GetResolveAttachmentReferences()[ColorAttachment], 0));
			}
		}

		// tonemap subpass non-msaa uses an additional color attachment that should not be used by main and depth subpasses
		if (bMobileTonemapSubpass && (NumColorAttachments > 1))
		{
			NumColorAttachments--;
		}

		if (RTLayout.GetDepthStencilAttachmentReference() != nullptr)
		{
			DepthStencilAttachmentReference = TAttachmentReferenceClass(*RTLayout.GetDepthStencilAttachmentReference(), 0);
		}

		// main sub-pass
		{
			TSubpassDescriptionClass& SubpassDesc = SubpassDescriptions[NumSubpasses++];

			SubpassDesc.SetColorAttachments(ColorAttachmentReferences, NumColorAttachments);
			if (!bDepthReadSubpass && !bMobileTonemapSubpass)
			{
				// only set resolve attachment on the last subpass
				SubpassDesc.SetResolveAttachments(ResolveAttachmentReferences);
			}
			if (RTLayout.GetDepthStencilAttachmentReference() != nullptr)
			{
				SubpassDesc.SetDepthStencilAttachment(&DepthStencilAttachmentReference);
			}

#if VULKAN_SUPPORTS_RENDERPASS2
#if VULKAN_SUPPORTS_DEPTH_STENCIL_RESOLVE
			if (!bDepthReadSubpass && bResolveDepth)
			{
				SubpassDesc.SetDepthStencilResolveAttachment(&DepthStencilResolveSubpassDesc);
			}
#endif // VULKAN_SUPPORTS_DEPTH_STENCIL_RESOLVE

#if VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE
			if (bApplyFragmentShadingRate)
			{
				SubpassDesc.SetShadingRateAttachment(&FragmentShadingRateAttachmentInfo);
			}
#endif // VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE
			SubpassDesc.SetMultiViewMask(MultiviewMask);
#endif // VULKAN_SUPPORTS_RENDERPASS2
		}

		// Color write and depth read sub-pass
		if (bDepthReadSubpass)
		{
			DepthStencilAttachmentOG.SetAttachment(*RTLayout.GetDepthStencilAttachmentReference(), VK_IMAGE_ASPECT_DEPTH_BIT);
			TSubpassDescriptionClass& SubpassDesc = SubpassDescriptions[NumSubpasses++];

			SubpassDesc.SetColorAttachments(ColorAttachmentReferences, NumColorAttachments);
			if (!bMobileTonemapSubpass)
			{
			SubpassDesc.SetResolveAttachments(ResolveAttachmentReferences);
			}

			check(RTLayout.GetDepthStencilAttachmentReference());

			// Depth as Input0
			InputAttachments1[0].SetAttachment(DepthStencilAttachmentOG, VK_IMAGE_ASPECT_DEPTH_BIT);
			InputAttachments1[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

			SubpassDesc.SetInputAttachments(InputAttachments1, InputAttachment1Count);
			// depth attachment is same as input attachment
			SubpassDesc.SetDepthStencilAttachment(InputAttachments1);

#if VULKAN_SUPPORTS_RENDERPASS2
#if VULKAN_SUPPORTS_DEPTH_STENCIL_RESOLVE
			if (bResolveDepth)
			{
				SubpassDesc.SetDepthStencilResolveAttachment(&DepthStencilResolveSubpassDesc);
			}
#endif // VULKAN_SUPPORTS_DEPTH_STENCIL_RESOLVE

#if VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE
			if (bApplyFragmentShadingRate)
			{
				SubpassDesc.SetShadingRateAttachment(&FragmentShadingRateAttachmentInfo);
			}
#endif // VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE
			SubpassDesc.SetMultiViewMask(MultiviewMask);
#endif // VULKAN_SUPPORTS_RENDERPASS2

			TSubpassDependencyClass& SubpassDep = SubpassDependencies[NumDependencies++];
			SubpassDep.srcSubpass = SrcSubpass++;
			SubpassDep.dstSubpass = DstSubpass++;
			SubpassDep.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			SubpassDep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			SubpassDep.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			SubpassDep.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			SubpassDep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}

		// Two subpasses for deferred shading
		if (bDeferredShadingSubpass)
		{
			// both sub-passes only test DepthStencil
			DepthStencilAttachment.attachment = RTLayout.GetDepthStencilAttachmentReference()->attachment;
			DepthStencilAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			DepthStencilAttachment.SetAspect(VK_IMAGE_ASPECT_DEPTH_BIT);	// @todo?

			//const VkAttachmentReference* ColorRef = RTLayout.GetColorAttachmentReferences();
			//uint32 NumColorAttachments = RTLayout.GetNumColorAttachments();
			//check(RTLayout.GetNumColorAttachments() == 5); //current layout is SceneColor, GBufferA/B/C/D

			// 1. Write to SceneColor and GBuffer, input DepthStencil
			{
				TSubpassDescriptionClass& SubpassDesc = SubpassDescriptions[NumSubpasses++];
				SubpassDesc.SetColorAttachments(ColorAttachmentReferences);
				SubpassDesc.SetDepthStencilAttachment(&DepthStencilAttachment);
				SubpassDesc.SetInputAttachments(&DepthStencilAttachment, 1);

#if VULKAN_SUPPORTS_RENDERPASS2
#if VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE
				if (bApplyFragmentShadingRate)
				{
					SubpassDesc.SetShadingRateAttachment(&FragmentShadingRateAttachmentInfo);
				}
#endif // VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE
				SubpassDesc.SetMultiViewMask(MultiviewMask);
#endif // VULKAN_SUPPORTS_RENDERPASS2

				// Depth as Input0
				TSubpassDependencyClass& SubpassDep = SubpassDependencies[NumDependencies++];
				SubpassDep.srcSubpass = SrcSubpass++;
				SubpassDep.dstSubpass = DstSubpass++;
				SubpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				SubpassDep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				SubpassDep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				SubpassDep.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
				SubpassDep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}

			// 2. Write to SceneColor, input GBuffer and DepthStencil
			{
				TSubpassDescriptionClass& SubpassDesc = SubpassDescriptions[NumSubpasses++];
				SubpassDesc.SetColorAttachments(ColorAttachmentReferences, 1); // SceneColor only
				SubpassDesc.SetDepthStencilAttachment(&DepthStencilAttachment);

				// Depth as Input0
				InputAttachments2[0].attachment = DepthStencilAttachment.attachment;
				InputAttachments2[0].layout = DepthStencilAttachment.layout;
				InputAttachments2[0].SetAspect(VK_IMAGE_ASPECT_DEPTH_BIT);

				// SceneColor write only
				InputAttachments2[1].attachment = VK_ATTACHMENT_UNUSED;
				InputAttachments2[1].layout = VK_IMAGE_LAYOUT_UNDEFINED;
				InputAttachments2[1].SetAspect(0);

				// GBufferA/B/C/D as Input2/3/4/5
				int32 NumColorInputs = ColorAttachmentReferences.Num() - 1;
				for (int32 i = 2; i < (NumColorInputs + 2); ++i)
				{
					InputAttachments2[i].attachment = ColorAttachmentReferences[i - 1].attachment;
					InputAttachments2[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					InputAttachments2[i].SetAspect(VK_IMAGE_ASPECT_COLOR_BIT);
				}

				SubpassDesc.SetInputAttachments(InputAttachments2, NumColorInputs + 2);
#if VULKAN_SUPPORTS_RENDERPASS2
#if VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE
				if (bApplyFragmentShadingRate)
				{
					SubpassDesc.SetShadingRateAttachment(&FragmentShadingRateAttachmentInfo);
				}
#endif // VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE
				SubpassDesc.SetMultiViewMask(MultiviewMask);
#endif // VULKAN_SUPPORTS_RENDERPASS2

				TSubpassDependencyClass& SubpassDep = SubpassDependencies[NumDependencies++];
				SubpassDep.srcSubpass = SrcSubpass++;
				SubpassDep.dstSubpass = DstSubpass++;
				SubpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				SubpassDep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				SubpassDep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				SubpassDep.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
				if (GVulkanInputAttachmentShaderRead == 1)
				{
					// this is not required, but might flicker on some devices without
					SubpassDep.dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
				}
				SubpassDep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}
		}

		// Tone Mapping Subpass
		if (bMobileTonemapSubpass)
		{
			TSubpassDescriptionClass& SubpassDesc = SubpassDescriptions[NumSubpasses++];

			InputAttachments3[0].attachment = VK_ATTACHMENT_UNUSED; // The subpass fetch logic expects depth in first attachment.
			InputAttachments3[0].layout = VK_IMAGE_LAYOUT_UNDEFINED;
			InputAttachments3[0].SetAspect(0);

			InputAttachments3[1].attachment = ColorAttachmentReferences[0].attachment;
			InputAttachments3[1].layout = VK_IMAGE_LAYOUT_GENERAL;
			InputAttachments3[1].SetAspect(VK_IMAGE_ASPECT_COLOR_BIT);

			if (RTLayout.GetHasResolveAttachments())
			{
				// MSAA or editor
				ColorAttachments3[0].attachment = ResolveAttachmentReferences[0].attachment;
			}
			else
			{
				// non-MSAA on device
				ColorAttachments3[0].attachment = ColorAttachmentReferences[1].attachment;
			}

			ColorAttachments3[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			ColorAttachments3[0].SetAspect(VK_IMAGE_ASPECT_COLOR_BIT);

			SubpassDesc.SetInputAttachments(InputAttachments3, 2);
			SubpassDesc.colorAttachmentCount = 1;
			SubpassDesc.pColorAttachments = ColorAttachments3;

#if VULKAN_SUPPORTS_RENDERPASS2
			if (bApplyFragmentShadingRate)
			{
				SubpassDesc.SetShadingRateAttachment(&FragmentShadingRateAttachmentInfo);
			}
			SubpassDesc.SetMultiViewMask(MultiviewMask);
#endif

#if VULKAN_SUPPORTS_QCOM_RENDERPASS_SHADER_RESOLVE
			// Combine the last Render and Store operations if possible
			if (Device.GetOptionalExtensions().HasQcomRenderPassShaderResolve && RTLayout.GetHasResolveAttachments())
			{
				SubpassDesc.flags |= VK_SUBPASS_DESCRIPTION_SHADER_RESOLVE_BIT_QCOM;
			}
#endif

			TSubpassDependencyClass& SubpassDep = SubpassDependencies[NumDependencies++];
			SubpassDep.srcSubpass = SrcSubpass++;
			SubpassDep.dstSubpass = DstSubpass++;
			SubpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			SubpassDep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			SubpassDep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			SubpassDep.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			SubpassDep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}

		ensure(DstSubpass == NumSubpasses);

		for (uint32 Attachment = 0; Attachment < RTLayout.GetNumAttachmentDescriptions(); ++Attachment)
		{
			AttachmentDescriptions.Add(TAttachmentDescriptionClass(RTLayout.GetAttachmentDescriptions()[Attachment]));
		}

		CreateInfo.attachmentCount = AttachmentDescriptions.Num();
		CreateInfo.pAttachments = AttachmentDescriptions.GetData();
		CreateInfo.subpassCount = NumSubpasses;
		CreateInfo.pSubpasses = SubpassDescriptions;
		CreateInfo.dependencyCount = NumDependencies;
		CreateInfo.pDependencies = SubpassDependencies;

		/*
		Bit mask that specifies which view rendering is broadcast to
		0011 = Broadcast to first and second view (layer)
		*/
		const uint32_t ViewMask[2] = { MultiviewMask, MultiviewMask };

		/*
		Bit mask that specifices correlation between views
		An implementation may use this for optimizations (concurrent render)
		*/
		CorrelationMask = MultiviewMask;

		VkRenderPassMultiviewCreateInfo MultiviewInfo;
		if (RTLayout.GetIsMultiView())
		{
#if VULKAN_SUPPORTS_RENDERPASS2
			if (Device.GetOptionalExtensions().HasKHRRenderPass2)
			{
				CreateInfo.SetCorrelationMask(&CorrelationMask);
			}
			else
#endif
			{
				checkf(Device.GetOptionalExtensions().HasKHRMultiview, TEXT("Layout is multiview but extension is not supported!"))
				ZeroVulkanStruct(MultiviewInfo, VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO);
				MultiviewInfo.subpassCount = NumSubpasses;
				MultiviewInfo.pViewMasks = ViewMask;
				MultiviewInfo.dependencyCount = 0;
				MultiviewInfo.pViewOffsets = nullptr;
				MultiviewInfo.correlationMaskCount = 1;
				MultiviewInfo.pCorrelationMasks = &CorrelationMask;

				MultiviewInfo.pNext = CreateInfo.pNext;
				CreateInfo.pNext = &MultiviewInfo;
			}
		}

		if (Device.GetOptionalExtensions().HasEXTFragmentDensityMap && RTLayout.GetHasFragmentDensityAttachment())
		{
			FragDensityCreateInfo.fragmentDensityMapAttachment = *RTLayout.GetFragmentDensityAttachmentReference();

			// Chain fragment density info onto create info and the rest of the pNexts
			// onto the fragment density info
			FragDensityCreateInfo.pNext = CreateInfo.pNext;
			CreateInfo.pNext = &FragDensityCreateInfo;
		}

#if VULKAN_SUPPORTS_QCOM_RENDERPASS_TRANSFORM
		if (RTLayout.GetQCOMRenderPassTransform() != VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			CreateInfo.flags = VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM;
		}
#endif
	}

	VkRenderPass Create(const FVulkanRenderTargetLayout& RTLayout)
	{
		BuildCreateInfo(RTLayout);

		return CreateInfo.Create(Device);
	}

	TRenderPassCreateInfoClass& GetCreateInfo()
	{
		return CreateInfo;
	}

private:
	TSubpassDescriptionClass SubpassDescriptions[8];
	TSubpassDependencyClass SubpassDependencies[8];

	TArray<TAttachmentReferenceClass> ColorAttachmentReferences;
	TArray<TAttachmentReferenceClass> ResolveAttachmentReferences;

	// Color write and depth read sub-pass
	static const uint32 InputAttachment1Count = 1;
	TAttachmentReferenceClass InputAttachments1[InputAttachment1Count];
	TAttachmentReferenceClass DepthStencilAttachmentOG;

	// Two subpasses for deferred shading
	TAttachmentReferenceClass InputAttachments2[MaxSimultaneousRenderTargets + 1];
	TAttachmentReferenceClass DepthStencilAttachment;

	TAttachmentReferenceClass DepthStencilAttachmentReference;
	TArray<TAttachmentDescriptionClass> AttachmentDescriptions;

	// Tonemap subpass
	TAttachmentReferenceClass InputAttachments3[MaxSimultaneousRenderTargets + 1];
	TAttachmentReferenceClass ColorAttachments3[MaxSimultaneousRenderTargets + 1];

#if VULKAN_SUPPORTS_RENDERPASS2
#if VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE
	FVulkanAttachmentReference<VkAttachmentReference2> ShadingRateAttachmentReference;
	FVulkanFragmentShadingRateAttachmentInfo FragmentShadingRateAttachmentInfo;
#endif // VULKAN_SUPPORTS_FRAGMENT_SHADING_RATE

#if VULKAN_SUPPORTS_DEPTH_STENCIL_RESOLVE
	FVulkanAttachmentReference<VkAttachmentReference2> DepthStencilResolveAttachmentReference;
	FVulkanDepthStencilResolveSubpassDesc DepthStencilResolveSubpassDesc;
#endif // VULKAN_SUPPORTS_DEPTH_STENCIL_RESOLVE
#endif // VULKAN_SUPPORTS_RENDERPASS2
	FVulkanRenderPassFragmentDensityMapCreateInfoEXT FragDensityCreateInfo;

	TRenderPassCreateInfoClass CreateInfo;
	FVulkanDevice& Device;

	uint32_t CorrelationMask;
};

VkRenderPass CreateVulkanRenderPass(FVulkanDevice& Device, const FVulkanRenderTargetLayout& RTLayout);

