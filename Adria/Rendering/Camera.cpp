#include <algorithm>
#include "Camera.h"
#include "Core/Input.h"
#include "Math/Constants.h"
#include "Math/Halton.h"
#include "my.h"
#include <SimpleMath.h>

using namespace DirectX;

namespace adria
{
	Camera::Camera(CameraParameters const& desc) 
		: position(desc.position), aspect_ratio(1.0f), fov(desc.fov), near_plane(desc.far_plane), far_plane(desc.near_plane), enabled(true), changed(false)
	{
		Vector3 look_vector = desc.look_at - position;
		look_vector.Normalize();

		Float yaw = std::atan2(look_vector.x, look_vector.z);
		Float pitch = std::asin(std::clamp(-look_vector.y, -1.0f, 1.0f));
		Quaternion pitch_quat = Quaternion::CreateFromYawPitchRoll(0, pitch, 0);
		Quaternion yaw_quat = Quaternion::CreateFromYawPitchRoll(yaw, 0, 0);
		orientation = pitch_quat * orientation * yaw_quat;
	}

	Vector3 Camera::Forward() const
	{
		return Vector3::Transform(Vector3::Forward, orientation);
	}

	Vector2 Camera::Jitter(Uint32 frame_index) const
	{
		Vector2 jitter{};
		constexpr HaltonSequence<16, 2> x;
		constexpr HaltonSequence<16, 3> y;
		jitter.x = x[frame_index % 16] - 0.5f;
		jitter.y = y[frame_index % 16] - 0.5f;
		return jitter;
	}
	Float Camera::Near() const
	{
		return near_plane;
	}
	Float Camera::Far() const
	{
		return far_plane;
	}
	Float Camera::Fov() const
	{
		return fov;
	}
	Float Camera::AspectRatio() const
	{
		return aspect_ratio;
	}

	void Camera::Update(Float dt, MyPluginRuntime*  p_runtime, MyWasmModuleId* module_id)
	{
		changed = false;
		if (!enabled || g_Input.GetKey(KeyCode::Space))
		{
			return;
		}

		// this should probably not be in camera logic
		Float x = g_Input.GetMousePositionX();
		Float y = g_Input.GetMousePositionY();
		if (g_Input.GetKey(KeyCode::MouseLeft))
		{
			if (!mouse_active) {
				p_runtime->TriggerEventPointerDownToRelevantSurface(MyPointerEvent {x, y});
			}
			mouse_active = true;
			p_runtime->TriggerEventPointerMoveToRelevantSurface(MyPointerEvent {x, y});
		} else {
			if (mouse_active) {
				mouse_active = false;
				p_runtime->TriggerEventPointerUpToRelevantSurface(MyPointerEvent {x, y});
			}
		}

		if (g_Input.GetKey(KeyCode::MouseRight))
		{
			Float dx = g_Input.GetMouseDeltaX();
			Float dy = g_Input.GetMouseDeltaY();
			Quaternion pitch_quat = Quaternion::CreateFromYawPitchRoll(0, dy * dt * 0.25f, 0);
			Quaternion yaw_quat = Quaternion::CreateFromYawPitchRoll(dx * dt * 0.25f, 0, 0);
			orientation = pitch_quat * orientation * yaw_quat;
			changed = true;
		}

		Vector3 movement{};
		if (g_Input.GetKey(KeyCode::W)) movement.z += 1.0f;
		if (g_Input.GetKey(KeyCode::S)) movement.z -= 1.0f;
		if (g_Input.GetKey(KeyCode::D)) movement.x += 1.0f;
		if (g_Input.GetKey(KeyCode::A)) movement.x -= 1.0f;
		if (g_Input.GetKey(KeyCode::Q)) movement.y += 1.0f;
		if (g_Input.GetKey(KeyCode::E)) movement.y -= 1.0f;
		movement = Vector3::Transform(movement, orientation);
		velocity = Vector3::SmoothStep(velocity, movement, 0.35f);

		if (velocity.LengthSquared() > 1e-4)
		{
			Float speed_factor = 1.0f;
			if (g_Input.GetKey(KeyCode::ShiftLeft)) speed_factor *= 5.0f;
			if (g_Input.GetKey(KeyCode::CtrlLeft))  speed_factor *= 0.2f;
			position += velocity * dt * speed_factor * 25.0f;
			changed = true;
		}

		auto new_cam_pos = p_runtime->PullSetCameraRequest();
        if (new_cam_pos != nullptr) {
			orientation = DirectX::SimpleMath::Quaternion(new_cam_pos->orientation.x, new_cam_pos->orientation.y, new_cam_pos->orientation.z, new_cam_pos->orientation.w);
			position = DirectX::SimpleMath::Vector3(new_cam_pos->position.x, new_cam_pos->position.y, new_cam_pos->position.z);
        }
		
		Matrix view_inverse = Matrix::CreateFromQuaternion(orientation) * Matrix::CreateTranslation(position);
		view_inverse.Invert(view_matrix);
		SetProjectionMatrix(fov, aspect_ratio, near_plane, far_plane);

		
		MyQuaternion orientation_ffi = {};
		orientation_ffi.x = orientation.x;
		orientation_ffi.y = orientation.y;
		orientation_ffi.z = orientation.z;
		orientation_ffi.w = orientation.w;
		MyVec3 position_ffi = {};
		position_ffi.x = position.x;
		position_ffi.y = position.y;
		position_ffi.z = position.z;
		MyCamera camera = {};
		camera.orientation = orientation_ffi;
		camera.position = position_ffi;
	    p_runtime->TriggerEventCameraOrientation(module_id, camera);

	}
	void Camera::Zoom(Int32 increment)
	{
		if (!enabled) return;
		fov -= XMConvertToRadians(increment * 1.0f);
		fov = std::clamp(fov, 0.00005f, pi_div_2<Float>);
		SetProjectionMatrix(fov, aspect_ratio, near_plane, far_plane);
	}
	void Camera::OnResize(Uint32 w, Uint32 h)
	{
		SetAspectRatio(static_cast<Float>(w) / h);
	}

	void Camera::SetAspectRatio(Float ar)
	{
		aspect_ratio = ar;
		SetProjectionMatrix(fov, aspect_ratio, near_plane, far_plane);
	}
	void Camera::SetFov(Float _fov)
	{
		fov = _fov;
		SetProjectionMatrix(fov, aspect_ratio, near_plane, far_plane);
	}
	void Camera::SetNearAndFar(Float n, Float f)
	{
		near_plane = n;
		far_plane = f;
		SetProjectionMatrix(fov, aspect_ratio, near_plane, far_plane);
	}
	void Camera::SetPosition(Vector3 const& pos)
	{
		position = pos;
	}

	Matrix Camera::View() const
	{
		return view_matrix;
	}
	Matrix Camera::Proj() const
	{
		return projection_matrix;
	}
	Matrix Camera::ViewProj() const
	{
		return view_matrix * projection_matrix;
	}
	BoundingFrustum Camera::Frustum() const
	{
		BoundingFrustum frustum(Proj());
		if (frustum.Far < frustum.Near) std::swap(frustum.Far, frustum.Near);
		frustum.Transform(frustum, view_matrix.Invert());
		return frustum;
	}
	void Camera::SetProjectionMatrix(Float fov, Float aspect, Float zn, Float zf)
	{
		projection_matrix = XMMatrixPerspectiveFovLH(fov, aspect, zn, zf);
	}
}