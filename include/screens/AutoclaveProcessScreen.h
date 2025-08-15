#pragma once

#include "Screen.h"
#include "ui/widget/Label.h"

#define STEP_TYPE_HEATING   1
#define STEP_TYPE_COOLING   2
#define STEP_TYPE_HOLD      3


class AutoclaveProcessScreen : public Screen {
	private:
		Label labelStepType;
		Label labelTemperature;
		Label labelTemperatureUnit;
		Label labelTime;
		Label labelTimeUnit;
		Label labelStopButton;

        Timer timerNotification;
        bool isNotificationShown = true;

        int prevStepType = 0;

	public:
		AutoclaveProcessScreen() :
            timerNotification(5000),
			labelStepType(0, 0, "Нагрев"),
			labelTemperature(0, 1, "", 3),
			labelTemperatureUnit(3, 1, "°C", 2),
			labelTime(8, 1, "", 3),
			labelTimeUnit(11, 1, "мин", 3),
			labelStopButton(8, 0, "Чтобы остановить процесс нажмите кнопку")
		{
            labelTemperature.setAlignment(ALIGN_RIGHT);
            labelTime.setAlignment(ALIGN_RIGHT);

            labelStopButton.setScrollable(true);
            labelStopButton.setScrollSpeed(750);
            labelStopButton.setScrollBy(8);
		}

		~AutoclaveProcessScreen() {
		}

		void reset() override {
			labelStepType.setText("Нагрев");
            isNotificationShown = true;
            labelStopButton.setVisible(true);
		}

		void update1s() override {
			labelTemperature.setText(viewContext->getTemperature());
			labelTime.setText(viewContext->getViewTimeMinutes());

            if (isNotificationShown && timerNotification.ready()) {
                isNotificationShown = false;
                labelStopButton.setVisible(false);
            }

            if (viewContext->getStepType() != prevStepType) {
                prevStepType = viewContext->getStepType();
                switch (viewContext->getStepType()) {
                    case STEP_TYPE_HEATING:
                        labelStepType.setText("Нагрев");
                        break;
                    case STEP_TYPE_COOLING:
                        labelStepType.setText("Остывание");
                        break;
                    case STEP_TYPE_HOLD:
                        labelStepType.setText("Готовка");
                        break;
                }
            }
		}

		void draw(BufferedLcd &lcd) override {
			labelStepType.print(lcd);
			labelTemperature.print(lcd);
			labelTemperatureUnit.print(lcd);
			labelTime.print(lcd);
			labelTimeUnit.print(lcd);
            labelStopButton.print(lcd);
		}

		void setStatus(const char* status) {
			labelStepType.setText(status);
		}

        void onEncoderTurn2(bool isRight, bool isFast) override {
            isNotificationShown = !isNotificationShown;
            labelStopButton.setVisible(isNotificationShown);
            labelStopButton.resetScroll();
            timerNotification.reset();
        }

        void onEncoderButtonClick() override {
            gotoStopProcessConfirmScreen();
        }

};


