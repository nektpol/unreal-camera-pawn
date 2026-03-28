# Top-Down Camera Pawn

**Developed by Neko Creative Technologies**

A versatile RTS / top-down camera pawn for Unreal Engine 5, featuring smooth movement, edge scrolling, rotation, zoom, and actor-following. Fully controllable via an interface for easy integration in any project.

---

## Features

- **Smooth Camera Movement**
  - Configurable speed, acceleration, and deceleration
  - Supports movement relative to camera rotation

- **Edge Scrolling**
  - Move the camera when the mouse reaches the screen edge
  - Adjustable scroll speed and threshold
  - Option to make movement relative to camera rotation

- **Camera Rotation**
  - Smooth yaw rotation
  - Configurable rotation speed
  - Reset to default rotation

- **Camera Zoom**
  - Zoom via arm length or FOV
  - Smooth interpolation
  - Configurable min/max zoom and zoom speed
  - Optional tilt with zoom

- **Follow Actor**
  - Snap or interpolate to a target actor
  - Automatically disables manual movement when following

- **Camera Control Interface**
  - Easily enable/disable movement, edge scroll, rotation, and zoom without direct pawn references
  - Functions include:
    - `SetCameraMovementEnabled(bool bEnable)`
    - `SetEdgeScrollEnabled(bool bEnable)`
    - `SetRotationEnabled(bool bEnable)`
    - `SetZoomEnabled(bool bEnable)`
    - `ResetCameraRotation()`
    - `ResetCameraZoom()`
    - `SnapCameraToLocation(FVector NewLocation)`
    - `InterpCameraToLocation(FVector NewLocation, float Duration)`
    - `SetCameraMoveSpeed(float Speed)`
    - `SetRotationSpeed(float Speed)`
    - `SetZoomSpeed(float Speed)`
    - `SetMinMaxZoom(float InMinZoom, float InMaxZoom)`
    - `SetTiltZoomEnabled(bool bEnable, float InTiltAmount)`
    - `SetEdgeScrollSpeed(float Speed)`
    - `SetEdgeScrollThreshold(float Pixels)`
    - `SetCameraRelativeMovement(bool bEnable)`
    - `SetFollowActor(AActor* InFollowActor)`
    - `StopFollowingActor()`

---

## Installation

1. Copy the `TopDownCameraPawn` folder into your project’s `Plugins` directory:
YourProject/Plugins


2. Open your project in Unreal Engine 5.

3. Go to **Edit → Plugins**, locate `TopDownCameraPawn`, and enable it.

4. Restart the editor.

---

## Usage

1. **Add Pawn to Level**
- Place the `TDCP_CameraPawn` in your level.
- Set it as your default **Player Pawn** in the GameMode if needed.

2. **Bind Enhanced Input**
- Ensure your project uses **Enhanced Input**.
- Assign input actions for movement, rotation, zoom, and reset.

3. **Controlling the Camera**
- Use the interface `ITDCP_CameraInterface` for easy runtime control.
- Example:
  ```cpp
  if (ITDCP_CameraInterface* CameraInterface = Cast<ITDCP_CameraInterface>(CameraPawn))
  {
      CameraInterface->SetCameraMovementEnabled(true);
      CameraInterface->SetEdgeScrollEnabled(false);
      CameraInterface->SetFollowActor(SomeActor);
  }
  ```

4. **Edge Scroll and Zoom Settings**
- Adjust speed, threshold, min/max zoom, and tilt in the pawn’s properties or via interface functions.

---

## Example

```cpp
// Enable movement and edge scroll
CameraPawn->SetCameraMovementEnabled(true);
CameraPawn->SetEdgeScrollEnabled(true);

// Zoom in smoothly
CameraPawn->HandleZoomInput(FInputActionValue(1.0f));

// Follow an actor
CameraPawn->SetFollowActor(TargetActor);
