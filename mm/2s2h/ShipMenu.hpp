#ifndef SHIPMENU_HPP
#define SHIPMENU_HPP

#include <functional>
#include <string>
#include <map>
#include <vector>
#include "2s2h/BenGui/UIWidgets.hpp"

struct ShipMod {
    std::function<void()> initFunc;
};

inline std::vector<std::function<void()>>& GetShipInitFuncs() {
    static std::vector<std::function<void()>> shipInitFuncs;
    return shipInitFuncs;
}

struct RegisterShipInitFunc {
    RegisterShipInitFunc(std::function<void()> initFunc) {
        GetShipInitFuncs().push_back(initFunc);
    }

    static void InitAll() {
        for (auto& initFunc : GetShipInitFuncs()) {
            initFunc();
        }
    }
};

struct ShipMenuItem {
    std::string name;
    std::string path;
    std::function<void(ShipMenuItem&)> drawFunc;
    std::function<void(ShipMenuItem&)> callbackFunc;
    std::set<std::string> children;

    void Draw() {
        if (drawFunc != nullptr) {
            drawFunc(*this);
        }
    }

    void DrawChildren() {
        auto& allShipMenuItems = GetAll();
        for (auto& childPath : children) {
            allShipMenuItems[childPath].Draw();
        }
    }

    static std::map<std::string, ShipMenuItem>& GetAll() {
        static std::map<std::string, ShipMenuItem> allShipMenuItems = {};
        return allShipMenuItems;
    }

    static std::set<std::string>& GetRootPaths() {
        static std::set<std::string> rootShipMenuItems = {};
        return rootShipMenuItems;
    }

    static ShipMenuItem* Get(const std::string& path) {
        auto& shipMenuItems = GetAll();

        auto it = shipMenuItems.find(path);
        if (it != shipMenuItems.end()) {
            return &it->second;
        }
        return nullptr;
    }
    
    static std::string GetParentPath(const std::string& path) {
        size_t pos = path.rfind('/');
        if (pos != std::string::npos) {
            return path.substr(0, pos);
        }
        return "";
    }

    static void InitializePath(const std::string& path) {
        auto& shipMenuItems = GetAll();

        std::string currentPath = path;
        while (true) {
            if (currentPath.empty()) {
                break;
            }

            auto& item = shipMenuItems[currentPath];
            item.path = currentPath;
            if (item.name.empty()) {
                item.name = currentPath.substr(currentPath.find_last_of('/') + 1);
                SPDLOG_INFO("ShipMenuItem at path {} has no name {}", currentPath, item.name);
            }

            std::string parentPath = GetParentPath(currentPath);
            if (parentPath.empty()) {
                // Add to the root items
                auto& rootShipMenuItems = GetRootPaths();
                rootShipMenuItems.insert(currentPath);
            } else {
                // Add the current item as a child of its parent
                auto& parentItem = shipMenuItems[parentPath];
                parentItem.children.insert(currentPath);
            }

            // Move to the next parent
            currentPath = parentPath;
        }
    }
};

struct ShipMenuCVarSliderInt {
  std::string name;
  std::string tooltip;
  std::string path;
  int min;
  int max;
  int defaultValue;
  std::function<std::string(ShipMenuItem&)> disabledReasonFunc;
  std::function<void(ShipMenuItem&)> callbackFunc;
};

struct ShipMenuCVarCheckbox {
    std::string name;
    std::string tooltip;
    std::string path;
    bool defaultValue;
    std::function<std::string(ShipMenuItem&)> disabledReasonFunc;
    std::function<void(ShipMenuItem&)> callbackFunc;
};

struct RegisterShipMenuItem {
    RegisterShipMenuItem(const ShipMenuItem& menuOption) {
        ShipMenuItem::InitializePath(menuOption.path);

        auto item = ShipMenuItem::Get(menuOption.path);

        if (!menuOption.name.empty()) {
            item->name = menuOption.name;
        }
        item->drawFunc = menuOption.drawFunc;
        item->callbackFunc = menuOption.callbackFunc;
    }

    RegisterShipMenuItem(const ShipMenuCVarSliderInt& sliderOption) {
        ShipMenuItem::InitializePath(sliderOption.path);

        auto item = ShipMenuItem::Get(sliderOption.path);

        if (!sliderOption.name.empty()) {
            item->name = sliderOption.name;
        }
        item->callbackFunc = sliderOption.callbackFunc;
        item->drawFunc = [sliderOption](ShipMenuItem& menuItem) {
            std::string disabledReason;
            if (sliderOption.disabledReasonFunc != nullptr) {
                disabledReason = sliderOption.disabledReasonFunc(menuItem);
            }

            if (UIWidgets::CVarSliderInt(sliderOption.name.c_str(), 
                                        sliderOption.path.c_str(), 
                                        sliderOption.min, 
                                        sliderOption.max, 
                                        sliderOption.defaultValue, {
                                            .tooltip = sliderOption.tooltip.c_str(),
                                            .disabled = !disabledReason.empty(),
                                            .disabledTooltip = disabledReason.c_str(),
                                        })) {
                if (sliderOption.callbackFunc != nullptr) {
                    sliderOption.callbackFunc(menuItem);
                }
            }
        };
    }

    RegisterShipMenuItem(const ShipMenuCVarCheckbox& checkboxOption) {
        ShipMenuItem::InitializePath(checkboxOption.path);

        auto item = ShipMenuItem::Get(checkboxOption.path);

        if (!checkboxOption.name.empty()) {
            item->name = checkboxOption.name;
        }
        item->callbackFunc = checkboxOption.callbackFunc;
        item->drawFunc = [checkboxOption](ShipMenuItem& menuItem) {
            std::string disabledReason;
            if (checkboxOption.disabledReasonFunc != nullptr) {
                disabledReason = checkboxOption.disabledReasonFunc(menuItem);
            }

            if (UIWidgets::CVarCheckbox(checkboxOption.name.c_str(), 
                                        checkboxOption.path.c_str(), {
                                            .tooltip = checkboxOption.tooltip.c_str(),
                                            .disabled = !disabledReason.empty(),
                                            .disabledTooltip = disabledReason.c_str(),
                                            .defaultValue = checkboxOption.defaultValue,
                                        })) {
                if (checkboxOption.callbackFunc != nullptr) {
                    checkboxOption.callbackFunc(menuItem);
                }
            }
        };
    }
};

#endif // SHIPMENU_HPP
