#pragma once

#include "Screen.h"
#include "ui/widget/Label.h"


class AutoclaveProcessScreen : public Screen {
	private:
		Label labelStatus;
		Label labelTemperature;
		Label labelTemperatureUnit;
		Label labelTime;
		Label labelTimeUnit;
		Label labelStopButton;

        Timer timerNotification;
        bool isNotificationShown = false;

	public:
		AutoclaveProcessScreen() :
            timerNotification(5000),
			labelStatus(0, 0, "Нагрев"),
			labelTemperature(0, 1, "", 3),
			labelTemperatureUnit(3, 1, "°C", 2),
			labelTime(8, 1, "", 3),
			labelTimeUnit(11, 1, "мин", 3),
			labelStopButton(8, 0, "Чтобы остановить процесс, нажмите кнопку")
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
			labelStatus.setText("Нагрев");
		}

		void update1s() override {
			labelTemperature.setText(viewContext->getTemperature());
			labelTime.setText(viewContext->getViewTimeMinutes());

            if (isNotificationShown && timerNotification.ready()) {
                isNotificationShown = false;
                labelStopButton.setVisible(false);
            }
		}

		void draw(BufferedLcd &lcd) override {
			labelStatus.print(lcd);
			labelTemperature.print(lcd);
			labelTemperatureUnit.print(lcd);
			labelTime.print(lcd);
			labelTimeUnit.print(lcd);
            labelStopButton.print(lcd);
		}

		void setStatus(const char* status) {
			labelStatus.setText(status);
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


