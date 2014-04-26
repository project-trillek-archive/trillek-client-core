#include "engine/core/TrillekGame.h"

int main(int argCount, char **argValues) {
    trillek::TrillekGame::GetScheduler().Initialize(5);
    return 0;
}
