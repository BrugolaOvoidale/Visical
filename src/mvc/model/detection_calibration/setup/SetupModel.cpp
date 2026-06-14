#include "SetupModel.hpp"
#include <UtilityFunctions.hpp>
#include <parameter/ParameterUtils.hpp>
#include <calibration/CameraIntrinsicsRegistry.hpp>
#include <calibration/DistortionModelRegistry.hpp>
#include "../SharedCameraIntrinsics.hpp"


SetupModel::SetupModel()
{
	init();

	automaticMode();
}

SetupModel::SetupModel(const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics)
{
	if (!camIntrinsics)
	{
		throw std::invalid_argument("camIntrinsics is nullptr");
	}

	camIntrinsics_ = camIntrinsics;

	init();
}

///////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<SetupModel> SetupModel::create()
{
	struct Enabler : SetupModel
	{
		Enabler() : SetupModel()
		{
		}
	};

	return std::make_shared<Enabler>();
}

std::shared_ptr<SetupModel> SetupModel::create(const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics)
{
	struct Enabler : SetupModel
	{
		Enabler(const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics) : SetupModel(camIntrinsics)
		{
		}
	};

	return std::make_shared<Enabler>(camIntrinsics);
}

///////////////////////////////////////////////////////////////////////////////////////

TaskResult SetupModel::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    const std::string& value)
{
	TaskResult setRes = ParameterOwner::setParameter(paramId, categoryId, value);

	if (setRes.isSuccess() && !paramsBatchUpdating_)
	{
		buildCamIntrinsics();
	}

    return setRes;
}

TaskResult SetupModel::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    int value)
{
	TaskResult setRes = ParameterOwner::setParameter(paramId, categoryId, value);

	if (setRes.isSuccess() && !paramsBatchUpdating_)
	{
		buildCamIntrinsics();
	}

	return setRes;
}

TaskResult SetupModel::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    double value)
{
	TaskResult setRes = ParameterOwner::setParameter(paramId, categoryId, value);

	if (setRes.isSuccess() && !paramsBatchUpdating_)
	{
		buildCamIntrinsics();
	}

	return setRes;
}

TaskResult SetupModel::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    bool value)
{
	TaskResult setRes = ParameterOwner::setParameter(paramId, categoryId, value);

	if (setRes.isSuccess() && !paramsBatchUpdating_)
	{
		buildCamIntrinsics();
	}

	return setRes;
}

TaskResult SetupModel::setParameter(
	const std::string& paramId,
	const std::string& categoryId,
	const std::variant<std::monostate, std::string, int, double, bool>& value)
{
	TaskResult setRes = ParameterOwner::setParameter(paramId, categoryId, value);

	if (setRes.isSuccess() && !paramsBatchUpdating_)
	{
		buildCamIntrinsics();
	}

	return setRes;
}

TaskResult SetupModel::setParameters(const std::vector<DTO>& params)
{
	paramsBatchUpdating_ = true;

	TaskResult setRes = ParameterOwner::setParameters(params);

	paramsBatchUpdating_ = false;

	if (setRes.isSuccess())
	{
		buildCamIntrinsics();
	}

	return setRes;
}

TaskResult SetupModel::resetParameter(
    const std::string& paramId,
    const std::string& categoryId)
{
	TaskResult resetRes = ParameterOwner::resetParameter(paramId, categoryId);

	if (resetRes.isSuccess())
	{
		buildCamIntrinsics();
	}

	return resetRes;
}

TaskResult SetupModel::resetDirtyParameters()
{
	paramsBatchUpdating_ = true;

	TaskResult resetRes = ParameterOwner::resetDirtyParameters();

	paramsBatchUpdating_ = false;

	if (resetRes.isSuccess())
	{
		buildCamIntrinsics();
	}

	return resetRes;
}

TaskResult SetupModel::automaticMode()
{
	try
	{
		CameraIntrinsics camIntrinsics = CameraIntrinsics::automatic(
			getRequiredParameter("image_width", CameraIntrinsicsRegistry::CATEGORY)->getValue<int>(),
			getRequiredParameter("image_height", CameraIntrinsicsRegistry::CATEGORY)->getValue<int>(),
			UtilityFunctions::enumFromString<DistortionModel::Type>(
				getRequiredParameter("distortion_type", DistortionModelRegistry::CATEGORY)->getValue<std::string>()).value()
		);

		if (!camIntrinsics_)
		{
			camIntrinsics_ = std::make_shared<SharedCameraIntrinsics>(std::move(camIntrinsics));
		}
		else
		{
			camIntrinsics_->set(std::move(camIntrinsics));
		}

	}
	catch (const std::exception& ex)
	{
		return { false, { ex.what(), Log::Level::LVL_ERROR } };
	}

	return true;
}

TaskResult SetupModel::fromHardwareMode()
{
	try
	{
		CameraIntrinsics camIntrinsics = CameraIntrinsics::fromHardware(
			getRequiredParameter("image_width", CameraIntrinsicsRegistry::CATEGORY)->getValue<int>(),
			getRequiredParameter("image_height", CameraIntrinsicsRegistry::CATEGORY)->getValue<int>(),
			getRequiredParameter("focal_length", CameraIntrinsicsRegistry::CATEGORY)->getValue<double>(),
			getRequiredParameter("pixel_pitch_width", CameraIntrinsicsRegistry::CATEGORY)->getValue<double>(),
			getRequiredParameter("pixel_pitch_height", CameraIntrinsicsRegistry::CATEGORY)->getValue<double>(),
			UtilityFunctions::enumFromString<DistortionModel::Type>(
				getRequiredParameter("distortion_type", DistortionModelRegistry::CATEGORY)->getValue<std::string>()).value()
		);

		if (!camIntrinsics_)
		{
			camIntrinsics_ = std::make_shared<SharedCameraIntrinsics>(std::move(camIntrinsics));
		}
		else
		{
			camIntrinsics_->set(std::move(camIntrinsics));
		}
	}
	catch (const std::exception& ex)
	{
		return { false, { ex.what(), Log::Level::LVL_ERROR } };
	}
	
	return true;
}

TaskResult SetupModel::freeMode()
{
	try
	{
		const DistortionModel::Type distType = UtilityFunctions::enumFromString<DistortionModel::Type>(
			getRequiredParameter("distortion_type", DistortionModelRegistry::CATEGORY)->getValue<std::string>()
		).value();
	
		std::optional<DistortionModel> distModel;

		switch (distType)
		{
			case DistortionModel::Type::STANDARD:
				distModel = DistortionModel::standard(
					getRequiredParameter("k1", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("k2", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("p1", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("p2", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("k3", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>()
				);

				break;

			case DistortionModel::Type::RATIONAL:
				distModel = DistortionModel::rational(
					getRequiredParameter("k1", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("k2", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("p1", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("p2", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("k3", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("k4", DistortionModelRegistry::CATEGORY_RATIONAL)->getValue<double>(),
					getRequiredParameter("k5", DistortionModelRegistry::CATEGORY_RATIONAL)->getValue<double>(),
					getRequiredParameter("k6", DistortionModelRegistry::CATEGORY_RATIONAL)->getValue<double>()
				);

				break;

			case DistortionModel::Type::THIN_PRISM:
				distModel = DistortionModel::thinPrism(
					getRequiredParameter("k1", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("k2", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("p1", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("p2", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("k3", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("k4", DistortionModelRegistry::CATEGORY_RATIONAL)->getValue<double>(),
					getRequiredParameter("k5", DistortionModelRegistry::CATEGORY_RATIONAL)->getValue<double>(),
					getRequiredParameter("k6", DistortionModelRegistry::CATEGORY_RATIONAL)->getValue<double>(),
					getRequiredParameter("s1", DistortionModelRegistry::CATEGORY_THIN_PRISM)->getValue<double>(),
					getRequiredParameter("s2", DistortionModelRegistry::CATEGORY_THIN_PRISM)->getValue<double>(),
					getRequiredParameter("s3", DistortionModelRegistry::CATEGORY_THIN_PRISM)->getValue<double>(),
					getRequiredParameter("s4", DistortionModelRegistry::CATEGORY_THIN_PRISM)->getValue<double>()
				);

				break;

			case DistortionModel::Type::TILTED:
				distModel = DistortionModel::tilted(
					getRequiredParameter("k1", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("k2", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("p1", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("p2", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("k3", DistortionModelRegistry::CATEGORY_STANDARD)->getValue<double>(),
					getRequiredParameter("k4", DistortionModelRegistry::CATEGORY_RATIONAL)->getValue<double>(),
					getRequiredParameter("k5", DistortionModelRegistry::CATEGORY_RATIONAL)->getValue<double>(),
					getRequiredParameter("k6", DistortionModelRegistry::CATEGORY_RATIONAL)->getValue<double>(),
					getRequiredParameter("s1", DistortionModelRegistry::CATEGORY_THIN_PRISM)->getValue<double>(),
					getRequiredParameter("s2", DistortionModelRegistry::CATEGORY_THIN_PRISM)->getValue<double>(),
					getRequiredParameter("s3", DistortionModelRegistry::CATEGORY_THIN_PRISM)->getValue<double>(),
					getRequiredParameter("s4", DistortionModelRegistry::CATEGORY_THIN_PRISM)->getValue<double>(),
					getRequiredParameter("tx", DistortionModelRegistry::CATEGORY_TILTED)->getValue<double>(),
					getRequiredParameter("ty", DistortionModelRegistry::CATEGORY_TILTED)->getValue<double>()
				);

				break;
		}

		CameraIntrinsics camIntrinsics = CameraIntrinsics::free(
			getRequiredParameter("image_width", CameraIntrinsicsRegistry::CATEGORY)->getValue<int>(),
			getRequiredParameter("image_height", CameraIntrinsicsRegistry::CATEGORY)->getValue<int>(),
			getRequiredParameter("focal_length_x", CameraIntrinsicsRegistry::CATEGORY)->getValue<double>(),
			getRequiredParameter("focal_length_y", CameraIntrinsicsRegistry::CATEGORY)->getValue<double>(),
			getRequiredParameter("principal_point_x", CameraIntrinsicsRegistry::CATEGORY)->getValue<double>(),
			getRequiredParameter("principal_point_y", CameraIntrinsicsRegistry::CATEGORY)->getValue<double>(),
			distModel.value()
		);

		if (!camIntrinsics_)
		{
			camIntrinsics_ = std::make_shared<SharedCameraIntrinsics>(std::move(camIntrinsics));
		}
		else
		{
			camIntrinsics_->set(std::move(camIntrinsics));
		}
	}
	catch (const std::exception& ex)
	{
		return { false, { ex.what(), Log::Level::LVL_ERROR } };
	}

	return true;
}

std::shared_ptr<SharedCameraIntrinsics> SetupModel::getCamIntrinsics() const
{
	return camIntrinsics_;
}

std::vector<std::shared_ptr<ParameterInfo>> SetupModel::getCamIntrinsicsParameters() const
{
	std::vector<std::shared_ptr<ParameterInfo>> params;

	switch (camIntrinsics_->get()->mode())
	{
		case CameraIntrinsics::Mode::AUTOMATIC:
			params.push_back(getRequiredParameter("image_width", CameraIntrinsicsRegistry::CATEGORY));
			params.push_back(getRequiredParameter("image_height", CameraIntrinsicsRegistry::CATEGORY));

			break;

		case CameraIntrinsics::Mode::FROM_HARDWARE:
			params.push_back(getRequiredParameter("image_width", CameraIntrinsicsRegistry::CATEGORY));
			params.push_back(getRequiredParameter("image_height", CameraIntrinsicsRegistry::CATEGORY));
			params.push_back(getRequiredParameter("focal_length", CameraIntrinsicsRegistry::CATEGORY));
			params.push_back(getRequiredParameter("pixel_pitch_width", CameraIntrinsicsRegistry::CATEGORY));
			params.push_back(getRequiredParameter("pixel_pitch_height", CameraIntrinsicsRegistry::CATEGORY));

			break;

		case CameraIntrinsics::Mode::FREE:
			params.push_back(getRequiredParameter("image_width", CameraIntrinsicsRegistry::CATEGORY));
			params.push_back(getRequiredParameter("image_height", CameraIntrinsicsRegistry::CATEGORY));

			break;
	}

	return params;
}

std::vector<std::shared_ptr<ParameterInfo>> SetupModel::getDistortionModelParameters() const
{
	std::vector<std::shared_ptr<ParameterInfo>> params;

	switch (camIntrinsics_->get()->mode())
	{
		case CameraIntrinsics::Mode::AUTOMATIC:
			params.push_back(getRequiredParameter("distortion_type", DistortionModelRegistry::CATEGORY));

			break;

		case CameraIntrinsics::Mode::FROM_HARDWARE:
			params.push_back(getRequiredParameter("distortion_type", DistortionModelRegistry::CATEGORY));

			break;

		case CameraIntrinsics::Mode::FREE:
		{
			params.push_back(getRequiredParameter("distortion_type", DistortionModelRegistry::CATEGORY));

			const DistortionModel::Type distType = UtilityFunctions::enumFromString<DistortionModel::Type>(
				getRequiredParameter("distortion_type", DistortionModelRegistry::CATEGORY)->getValue<std::string>()
			).value();

			switch (distType)
			{
				case DistortionModel::Type::STANDARD:
					params.push_back(getRequiredParameter("k1", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("k2", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("p1", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("p2", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("k3", DistortionModelRegistry::CATEGORY_STANDARD));
					break;

				case DistortionModel::Type::RATIONAL:
					params.push_back(getRequiredParameter("k1", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("k2", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("p1", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("p2", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("k3", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("k4", DistortionModelRegistry::CATEGORY_RATIONAL));
					params.push_back(getRequiredParameter("k5", DistortionModelRegistry::CATEGORY_RATIONAL));
					params.push_back(getRequiredParameter("k6", DistortionModelRegistry::CATEGORY_RATIONAL));
					break;

				case DistortionModel::Type::THIN_PRISM:
					params.push_back(getRequiredParameter("k1", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("k2", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("p1", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("p2", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("k3", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("k4", DistortionModelRegistry::CATEGORY_RATIONAL));
					params.push_back(getRequiredParameter("k5", DistortionModelRegistry::CATEGORY_RATIONAL));
					params.push_back(getRequiredParameter("k6", DistortionModelRegistry::CATEGORY_RATIONAL));
					params.push_back(getRequiredParameter("s1", DistortionModelRegistry::CATEGORY_THIN_PRISM));
					params.push_back(getRequiredParameter("s2", DistortionModelRegistry::CATEGORY_THIN_PRISM));
					params.push_back(getRequiredParameter("s3", DistortionModelRegistry::CATEGORY_THIN_PRISM));
					params.push_back(getRequiredParameter("s4", DistortionModelRegistry::CATEGORY_THIN_PRISM));
					break;

				case DistortionModel::Type::TILTED:
					params.push_back(getRequiredParameter("k1", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("k2", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("p1", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("p2", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("k3", DistortionModelRegistry::CATEGORY_STANDARD));
					params.push_back(getRequiredParameter("k4", DistortionModelRegistry::CATEGORY_RATIONAL));
					params.push_back(getRequiredParameter("k5", DistortionModelRegistry::CATEGORY_RATIONAL));
					params.push_back(getRequiredParameter("k6", DistortionModelRegistry::CATEGORY_RATIONAL));
					params.push_back(getRequiredParameter("s1", DistortionModelRegistry::CATEGORY_THIN_PRISM));
					params.push_back(getRequiredParameter("s2", DistortionModelRegistry::CATEGORY_THIN_PRISM));
					params.push_back(getRequiredParameter("s3", DistortionModelRegistry::CATEGORY_THIN_PRISM));
					params.push_back(getRequiredParameter("s4", DistortionModelRegistry::CATEGORY_THIN_PRISM));
					params.push_back(getRequiredParameter("tx", DistortionModelRegistry::CATEGORY_TILTED));
					params.push_back(getRequiredParameter("ty", DistortionModelRegistry::CATEGORY_TILTED));
					break;
			}

			break;
		}
	}

	return params;
}

std::vector<std::shared_ptr<ParameterInfo>> SetupModel::getCamMatrixParameters() const
{
	std::vector<std::shared_ptr<ParameterInfo>> params;

	switch (camIntrinsics_->get()->mode())
	{
	case CameraIntrinsics::Mode::AUTOMATIC:

		break;

	case CameraIntrinsics::Mode::FROM_HARDWARE:

		break;

	case CameraIntrinsics::Mode::FREE:
		params.push_back(getRequiredParameter("focal_length_x", CameraIntrinsicsRegistry::CATEGORY));
		params.push_back(getRequiredParameter("focal_length_y", CameraIntrinsicsRegistry::CATEGORY));
		params.push_back(getRequiredParameter("principal_point_x", CameraIntrinsicsRegistry::CATEGORY));
		params.push_back(getRequiredParameter("principal_point_y", CameraIntrinsicsRegistry::CATEGORY));

		break;
	}

	return params;
}

///////////////////////////////////////////////////////////////////////////////////////

void SetupModel::init()
{
	std::vector<std::shared_ptr<ParameterInfo>> infoVec;

	CameraIntrinsicsRegistry camIntrinsicsRegistry;
	UtilityFunctions::moveInto(camIntrinsicsRegistry.getAllParameters(), infoVec);

	//
	DistortionModelRegistry distModelRegistry;
	UtilityFunctions::moveInto(distModelRegistry.getAllParameters(), infoVec);

	initParameters(infoVec);
}

void SetupModel::buildCamIntrinsics()
{
	switch (camIntrinsics_->get()->mode())
	{
	case CameraIntrinsics::Mode::AUTOMATIC:
		automaticMode();

		break;

	case CameraIntrinsics::Mode::FROM_HARDWARE:
		fromHardwareMode();

		break;

	case CameraIntrinsics::Mode::FREE:
		freeMode();

		break;
	}
}

std::shared_ptr<const ParameterOwner> SetupModel::getSharedParameterOwner() const
{
	return std::static_pointer_cast<const ParameterOwner>(shared_from_this());
}