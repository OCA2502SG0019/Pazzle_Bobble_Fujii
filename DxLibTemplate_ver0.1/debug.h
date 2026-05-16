#pragma once

class LaunchPadAnimation;
class ArrowAnimation;

class DebugManager {
public:
    DebugManager();
    void toggle();
    bool isEnabled() const;
    void draw(const LaunchPadAnimation& launch, const ArrowAnimation& arrow) const;

private:
    bool m_enabled;
};
