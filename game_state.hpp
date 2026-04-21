#pragma once

#include <array>
#include <exception>
#include <optional>

class InvalidOperation : public std::exception {
public:
    const char* what() const noexcept override {
        return "invalid operation";
    }
};

struct PlayInfo {
    int dummyCount = 0;
    int magnifierCount = 0;
    int converterCount = 0;
    int cageCount = 0;
};

class GameState {
public:
    enum class BulletType { Live, Blank };
    enum class ItemType { Dummy, Magnifier, Converter, Cage };

    GameState() {
        hp[0] = 5;
        hp[1] = 5;
        currentPlayer = 0;
        liveCount = 0;
        blankCount = 0;
        knownBullet = std::nullopt;
        cageActive = false;
        cageUsedThisTurn = false;
    }

    void fireAtOpponent(BulletType topBulletBeforeAction) {
        consumeBullet(topBulletBeforeAction);
        bool turnEnded = true;
        if (topBulletBeforeAction == BulletType::Live) {
            hp[1 - currentPlayer]--;
        }
        
        if (winnerId() != -1) return;

        if (cageActive) {
            cageActive = false;
            turnEnded = false;
        }

        if (turnEnded) {
            switchTurn();
        }
    }

    void fireAtSelf(BulletType topBulletBeforeAction) {
        consumeBullet(topBulletBeforeAction);
        bool turnEnded = false;
        if (topBulletBeforeAction == BulletType::Live) {
            hp[currentPlayer]--;
            turnEnded = true;
        } else {
            // Blank, continue turn
            turnEnded = false;
        }

        if (winnerId() != -1) return;

        if (turnEnded) {
            if (cageActive) {
                cageActive = false;
                turnEnded = false;
            }
        }

        if (turnEnded) {
            switchTurn();
        }
    }

    void useDummy(BulletType topBulletBeforeUse) {
        if (players[currentPlayer].dummyCount <= 0) throw InvalidOperation();
        players[currentPlayer].dummyCount--;
        consumeBullet(topBulletBeforeUse);
    }

    void useMagnifier(BulletType topBulletBeforeUse) {
        if (players[currentPlayer].magnifierCount <= 0) throw InvalidOperation();
        players[currentPlayer].magnifierCount--;
        knownBullet = topBulletBeforeUse;
    }

    void useConverter(BulletType topBulletBeforeUse) {
        if (players[currentPlayer].converterCount <= 0) throw InvalidOperation();
        players[currentPlayer].converterCount--;
        
        // Flip the bullet
        if (topBulletBeforeUse == BulletType::Live) {
            liveCount--;
            blankCount++;
            knownBullet = BulletType::Blank;
        } else {
            liveCount++;
            blankCount--;
            knownBullet = BulletType::Live;
        }
    }

    void useCage() {
        if (players[currentPlayer].cageCount <= 0) throw InvalidOperation();
        if (cageUsedThisTurn) throw InvalidOperation();
        players[currentPlayer].cageCount--;
        cageActive = true;
        cageUsedThisTurn = true;
    }

    void reloadBullets(int live, int blank) {
        liveCount = live;
        blankCount = blank;
        knownBullet = std::nullopt;
    }

    void reloadItem(int playerId, ItemType item) {
        if (item == ItemType::Dummy) players[playerId].dummyCount++;
        else if (item == ItemType::Magnifier) players[playerId].magnifierCount++;
        else if (item == ItemType::Converter) players[playerId].converterCount++;
        else if (item == ItemType::Cage) players[playerId].cageCount++;
    }

    double nextLiveBulletProbability() const {
        int total = liveCount + blankCount;
        if (total == 0) return 0.0;
        if (knownBullet.has_value()) {
            return (*knownBullet == BulletType::Live) ? 1.0 : 0.0;
        }
        return (double)liveCount / total;
    }

    double nextBlankBulletProbability() const {
        int total = liveCount + blankCount;
        if (total == 0) return 0.0;
        if (knownBullet.has_value()) {
            return (*knownBullet == BulletType::Blank) ? 1.0 : 0.0;
        }
        return (double)blankCount / total;
    }

    int winnerId() const {
        if (hp[0] <= 0) return 1;
        if (hp[1] <= 0) return 0;
        return -1;
    }

private:
    int hp[2];
    int currentPlayer;
    int liveCount;
    int blankCount;
    std::optional<BulletType> knownBullet;
    bool cageActive;
    bool cageUsedThisTurn;
    PlayInfo players[2];

    void consumeBullet(BulletType type) {
        if (type == BulletType::Live) liveCount--;
        else blankCount--;
        knownBullet = std::nullopt;
    }

    void switchTurn() {
        currentPlayer = 1 - currentPlayer;
        cageActive = false;
        cageUsedThisTurn = false;
    }
};
