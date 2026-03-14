#include "mui.hpp"
#include <iostream>

int main() {
    try {
        mui::App::init();

        // 1. Create the UI components
        mui::Window window("Modern C++ Wrapper", 400, 200);
        auto mainLayout = std::make_shared<mui::VBox>();
        auto inputLayout = std::make_shared<mui::HBox>();

        auto titleLabel = std::make_shared<mui::Label>("Enter your name:");
        auto nameEntry = std::make_shared<mui::Entry>();
        auto submitBtn = std::make_shared<mui::Button>("Say Hello");
        auto resultLabel = std::make_shared<mui::Label>("");

        // 2. Build the layout tree
        window.setMargined(true);
        mainLayout->setPadded(true);
        inputLayout->setPadded(true);

        inputLayout->append(titleLabel, false);
        inputLayout->append(nameEntry, true); // Stretchy

        mainLayout->append(inputLayout, false);
        mainLayout->append(submitBtn, false);
        mainLayout->append(resultLabel, false);

        window.setChild(mainLayout);

        // 3. Hook up modern C++ lambdas for logic
        submitBtn->onClick([&]() {
            std::string name = nameEntry->getText();
            if (name.empty()) {
                resultLabel->setText("Please enter a name!");
            } else {
                resultLabel->setText("Hello there, " + name + "!");
                nameEntry->setText(""); // clear the input
            }
        });

        window.onClosing([&]() {
            mui::App::quit();
            return true; // true tells libui to actually destroy the window
        });

        // 4. Run the app
        window.show();
        mui::App::run();

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}