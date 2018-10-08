#include "bnPlayerControlledState.h"
#include "bnInputManager.h"
#include "bnPlayer.h"
#include "bnTile.h"
#include "bnAudioResourceManager.h"

#include <iostream>

#define MOVE_KEY_PRESS_COOLDOWN 200.0f
#define MOVE_LAG_COOLDOWN 40.0f
#define ATTACK_KEY_PRESS_COOLDOWN 300.0f
#define ATTACK_TO_IDLE_COOLDOWN 150.0f
#define HIT_COOLDOWN 300.0f

PlayerControlledState::PlayerControlledState() : inputManager(&InputManager::GetInstance()), AIState<Player>()
{
  //Cooldowns. TODO: Take these out. We base actions on animation speed now.
  moveKeyPressCooldown = MOVE_KEY_PRESS_COOLDOWN;
  attackKeyPressCooldown = ATTACK_KEY_PRESS_COOLDOWN;
  attackToIdleCooldown = 0.0f;
  previousDirection = Direction::NONE;
  isChargeHeld = false;
}


PlayerControlledState::~PlayerControlledState()
{
  inputManager = nullptr;
}

void PlayerControlledState::OnEnter(Player& player) {
  player.SetAnimation(PLAYER_IDLE);
}

void PlayerControlledState::OnUpdate(float _elapsed, Player& player) {

  // Action controls take priority over movement
  if (inputManager->has(RELEASED_A)) {
    if (player.chargeComponent.GetChargeCounter() > 0 && isChargeHeld == true) {
      player.Attack(player.chargeComponent.GetChargeCounter());
      player.chargeComponent.SetCharging(false);
      isChargeHeld = false;
      attackKeyPressCooldown = 0.0f;
      auto onFinish = [&]() {player.SetAnimation(PLAYER_IDLE);};
      player.SetAnimation(PLAYER_SHOOTING, onFinish);
    }
    else {
      isChargeHeld = false;
    }
  }

  // Movement increments are restricted based on anim speed
  if (player.state != PLAYER_IDLE)
    return;

  static Direction direction = Direction::NONE;
  if (moveKeyPressCooldown >= MOVE_KEY_PRESS_COOLDOWN) {
    if (inputManager->has(PRESSED_UP)) {
      direction = Direction::UP;
    }
    else if (inputManager->has(PRESSED_LEFT)) {
      direction = Direction::LEFT;
    }
    else if (inputManager->has(PRESSED_DOWN)) {
      direction = Direction::DOWN;
    }
    else if (inputManager->has(PRESSED_RIGHT)) {
      direction = Direction::RIGHT;
    }
  }
 

  if (inputManager->has(PRESSED_A) && isChargeHeld == false) {
    player.Attack(player.chargeComponent.GetChargeCounter());
    isChargeHeld = true;

    attackKeyPressCooldown = 0.0f;
    auto onFinish = [&]() { 

      if (this->isChargeHeld) {
        player.chargeComponent.SetCharging(true);
      }

      player.SetAnimation(PLAYER_IDLE); 
      this->attackKeyPressCooldown = ATTACK_KEY_PRESS_COOLDOWN; 
    };
    player.SetAnimation(PLAYER_SHOOTING, onFinish);
  }

  if (inputManager->has(RELEASED_UP)) {
    direction = Direction::NONE;
  }
  else if (inputManager->has(RELEASED_LEFT)) {
    direction = Direction::NONE;
  }
  else if (inputManager->has(RELEASED_DOWN)) {
    direction = Direction::NONE;
  }
  else if (inputManager->has(RELEASED_RIGHT)) {
    direction = Direction::NONE;
  }

  if (!player.HasFloatShoe()) {
    if (player.GetTile()->GetState() == TileState::ICE) {
      if (previousDirection != Direction::NONE) {
        bool moved = player.Move(previousDirection);

        if (moved) {
          player.AdoptNextTile();
        }
        else {
          previousDirection = Direction::NONE;
        }
      }
    }
    else if (player.GetTile()->GetState() == TileState::LAVA) {
      // player.SetHealth(player.GetHealth() - 1);
    }
    else {
      previousDirection = Direction::NONE;
    }
  }

  if (direction != Direction::NONE && player.state != PLAYER_SHOOTING) {
    bool moved = player.Move(direction);

    previousDirection = direction;

    if (moved) {
      moveKeyPressCooldown = 0.0f;
      auto onFinish = [&]() {
        //Cooldown until player's movement catches up to actual position (avoid walking through spells)
        if (player.previous) {
          player.AdoptNextTile();
        }
        player.SetAnimation(PLAYER_IDLE);
        direction = Direction::NONE;
      }; // end lambda
      player.SetAnimation(PLAYER_MOVING, onFinish);
    }
    else {
      player.SetAnimation(PLAYER_IDLE);
    }
    moveKeyPressCooldown = MOVE_KEY_PRESS_COOLDOWN;
  }
}

void PlayerControlledState::OnLeave(Player& player) {
  /* Mega loses charge when we release control */
  player.chargeComponent.SetCharging(false);
}
