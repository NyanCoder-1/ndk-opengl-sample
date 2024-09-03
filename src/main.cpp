#include "Graphics.hpp"
#include <android/log.h>
#include <android_native_app_glue.h>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

struct MyUserData {
	bool wantToClose = false;
	Graphics::Ptr graphics;
};

int32_t OnInputEvent(struct android_app* pAndroidApp, AInputEvent* pEvent)
{
	auto userData = reinterpret_cast<MyUserData*>(pAndroidApp->userData);
	if (!userData)
		return 0;

	if (AInputEvent_getType(pEvent) == AINPUT_EVENT_TYPE_KEY) {
		if (AKeyEvent_getAction(pEvent) == AKEY_EVENT_ACTION_UP) {
			if (AKeyEvent_getKeyCode(pEvent) == AKEYCODE_BACK) {
				userData->wantToClose = true;
				return 1;
			}
		}
	}
	bool isMotion = AInputEvent_getType(pEvent) == AINPUT_EVENT_TYPE_MOTION;
	bool isTouchDown = (AMOTION_EVENT_ACTION_MASK & AMotionEvent_getAction(pEvent)) == AMOTION_EVENT_ACTION_DOWN;
	bool isTouchUp = (AMOTION_EVENT_ACTION_MASK & AMotionEvent_getAction(pEvent)) == AMOTION_EVENT_ACTION_UP;
	float xDevicePixel = AMotionEvent_getX(pEvent, 0);
	float yDevicePixel = AMotionEvent_getY(pEvent, 0);

	// OnHandleTouch(pEvent, isMotion, isTouchDown, isTouchUp, xDevicePixel, yDevicePixel);
	return 1;
}

/**
* application events: application on focus, lost focus, window create, window close
*/
void OnAppCmd(struct android_app* pAndroidApp, int32_t cmd)
{
	auto userData = reinterpret_cast<MyUserData*>(pAndroidApp->userData);
	if (!userData)
		return;

	switch (cmd) {
		case APP_CMD_SAVE_STATE:
			break;

		case APP_CMD_INIT_WINDOW:
			if (userData->graphics)
				userData->graphics->OnCreateWindow(pAndroidApp->window);
			break;

		case APP_CMD_TERM_WINDOW:
			if (userData->graphics)
				userData->graphics->OnKillWindow();
			break;

		case APP_CMD_GAINED_FOCUS:
			// OnActiveFocus
			break;

		case APP_CMD_LOST_FOCUS:
			// OnLostFocus
			break;

		default:
			break;
	}
}

class MyDefer {
public:
	MyDefer() = delete;
	explicit MyDefer(const std::function<void()> &fn) : callback(fn) {}
	~MyDefer()
	{
		if (callback)
			callback();
	}

private:
	std::function<void()> callback;
};

/**
* This is the main entry point of a native application that is using android_native_app_glue.
* It runs in its own thread, with its own event loop for receiving input events and draw scene.
*/
void android_main(struct android_app* pAndroidApp)
{
	pAndroidApp->onAppCmd = OnAppCmd;
	pAndroidApp->onInputEvent = OnInputEvent;

	auto userData = new MyUserData();
	userData->graphics = Graphics::Create();
	pAndroidApp->userData = reinterpret_cast<void*>(userData);
	auto freeUserData = MyDefer([&pAndroidApp, &userData]() {
		if (pAndroidApp->userData) {
			delete userData;
			userData = nullptr;
			pAndroidApp->userData = nullptr;
		}
	});

	// events.
	int events;
	struct android_poll_source* source;

	// loop waiting for stuff to do.
	while (true)
	{
		if (ALooper_pollOnce(0, nullptr, &events,(void**)&source) >= 0)
		{
			// Process this event.
			if (source) {
				source->process(pAndroidApp, source);
			}

			// stop application
			if (pAndroidApp->destroyRequested)
			{
				if (userData->graphics)
					userData->graphics->OnKillWindow();
				return;
			}
		}

		// Update & OnDraw
		if (userData->graphics)
			userData->graphics->OnDraw();

		if (userData->wantToClose)
			ANativeActivity_finish(pAndroidApp->activity);
	}
}
