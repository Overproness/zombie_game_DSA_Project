# SurvivalGame

A zombie survival game built with Unreal Engine 4.27, featuring cooperative gameplay, AI-driven enemies, and extensive modding support.

## Overview

SurvivalGame is a third-person survival game where players must survive against waves of zombies while managing resources like hunger and health. The game features intelligent zombie AI, weapon systems, and supports both single-player and cooperative multiplayer modes.

## Features

### Gameplay
- **Zombie AI System**: Advanced AI with behavior trees and blackboards for realistic zombie behavior
- **Survival Mechanics**: Hunger system and health management
- **Weapon Systems**: Assault rifles and instant-hit weapons with realistic damage types
- **Physics-Based Combat**: Different damage zones (head, body, limbs) with unique physical materials
- **Time of Day System**: Dynamic day/night cycle
- **Noise Detection**: Zombies react to player-generated sounds

### Game Modes
- **Single Player**: Solo survival experience
- **Cooperative Multiplayer**: Team up with friends (Steam integration)
- **Open World Mode**: Explore larger environments
- **Default Mode**: Standard survival gameplay

### Technical Features
- Custom damage types and physical materials
- Particle effects for weapons and impacts
- Animation Blueprint with aim offsets and blend spaces
- Material highlighting system for interactive objects
- Post-processing effects with custom depth occlusion

## Requirements

- **Unreal Engine**: 4.27
- **Platform**: Windows (Win32/Win64)
- **Steam**: Required for multiplayer features
- **IDE**: Visual Studio 2019 or later (for C++ development)

## Installation

1. **Clone the repository:**
   ```bash
   git clone https://github.com/Overproness/zombie_game_DSA_Project.git
   cd zombie_game_DSA_Project
   ```

2. **Generate project files:**
   - Right-click on `SurvivalGame.uproject`
   - Select "Generate Visual Studio project files"

3. **Build the project:**
   - Open `SurvivalGame.sln` in Visual Studio
   - Set build configuration to `Development Editor`
   - Build the solution (Ctrl+Shift+B)

4. **Launch the editor:**
   - Double-click `SurvivalGame.uproject` or
   - Launch from Visual Studio (F5)

## Building for Distribution

### Development Build (Win64)
```bash
Engine\Build\BatchFiles\Build.bat SurvivalGame Win64 Development "C:\Path\To\SurvivalGame.uproject" -waitmutex
```

### Shipping Build (Win64)
```bash
Engine\Build\BatchFiles\Build.bat SurvivalGame Win64 Shipping "C:\Path\To\SurvivalGame.uproject" -waitmutex
```

### Server Build
```bash
Engine\Build\BatchFiles\Build.bat SurvivalGameServer Win64 Development "C:\Path\To\SurvivalGame.uproject" -waitmutex
```

## Project Structure

```
SurvivalGame/
├── Source/                  # C++ source code
│   └── SurvivalGame/       # Main game module
├── Content/                # Game assets
│   ├── AI/                 # Zombie AI (Behavior Trees, Blackboards)
│   ├── AnimStarterPack/    # Character animations
│   ├── Base/               # Core game blueprints and materials
│   ├── Effects/            # Particle systems and VFX
│   ├── Environment/        # Level assets and props
│   ├── Items/              # Pickups and inventory items
│   ├── Maps/               # Game levels
│   ├── Player/             # Player character assets
│   ├── Sound/              # Audio files
│   ├── UI/                 # User interface widgets
│   ├── Weapons/            # Weapon blueprints and assets
│   └── ZombieCharacters/   # Zombie models and materials
├── Config/                 # Game configuration files
├── Plugins/                # Game modifications
│   ├── ExtendedRifleMod/   # Extended rifle features
│   └── MyFlashlightMod/    # Flashlight system
└── Binaries/               # Compiled game files
```

## Key Systems

### AI System
- **Behavior Trees**: `ZombieBT.uasset` - Controls zombie decision-making
- **Blackboards**: `ZombieBlackboard.uasset` - Stores AI state data
- **Custom Tasks**: `Task_AttackMelee.uasset` - Melee attack behavior
- **Noise Emitter**: Zombies track and respond to player noise

### Damage System
- **DmgType_AssaultRifle**: High-velocity projectile damage
- **DmgType_InstantWeapon**: Hitscan weapon damage
- **DmgType_ZombieMelee**: Zombie attack damage
- **DmgType_Hunger**: Environmental damage from starvation

### Physical Materials
- **PhysMat_PlayerBody**: Player collision and damage
- **PhysMat_ZombieHead**: Critical hit zone
- **PhysMat_ZombieBody**: Standard damage zone
- **PhysMat_ZombieLimb**: Dismemberment system

### Animation System
- Aim offsets for hip and ironsight aiming
- Blend spaces for movement (walk, jog, crouch)
- Reload montages for weapons
- Player Animation Blueprint with state machine

## Modding Support

The game includes a plugin system for easy modification:

### Creating a Mod
1. Create a new plugin in the `Plugins/` folder
2. Add your custom blueprints, assets, or C++ code
3. Enable the plugin in the project settings
4. Package your mod for distribution

### Example Mods
- **ExtendedRifleMod**: Adds enhanced rifle mechanics
- **MyFlashlightMod**: Implements flashlight system

## Configuration

### Game Settings
- `Config/DefaultEngine.ini` - Engine and rendering settings
- `Config/DefaultGame.ini` - Game-specific settings
- `Config/DefaultInput.ini` - Input bindings
- `Config/DefaultEditor.ini` - Editor preferences

### Adjusting Difficulty
Modify zombie spawn rates, health, and damage in the respective game mode blueprints:
- `SurvivalGameModeSetup.uasset` - Standard mode
- `SurvivalCoopGameModeSetup.uasset` - Cooperative mode
- `OpenWorldGameModeSetup.uasset` - Open world mode

## Multiplayer Setup

The game uses Steam for networking:

1. Ensure Steam is running
2. OnlineSubsystemSteam plugin is enabled
3. Use the Coop game mode for multiplayer sessions
4. Configure Steam AppID in DefaultEngine.ini

## Development Tasks

### Available Build Tasks
- Debug, DebugGame, Development, Shipping, Test configurations
- Win32 and Win64 platform support
- Editor, Game, and Server targets
- Clean and Rebuild options

### Running in Editor
```bash
# Launch editor with specific map
UE4Editor.exe "C:\Path\To\SurvivalGame.uproject" MapName -game
```

## Troubleshooting

### Build Errors
- Ensure Unreal Engine 4.27 is properly installed
- Regenerate project files if solution is outdated
- Clean intermediate and binaries folders

### Runtime Issues
- Verify all required assets are present in Content folder
- Check log files in `Saved/Logs/`
- Ensure Steam is running for multiplayer

### Performance
- Adjust quality settings in DefaultEngine.ini
- Optimize particle counts in Effects folder
- Use profiling tools (Stat FPS, Stat Unit)

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## Credits

- **Engine**: Unreal Engine 4.27 by Epic Games
- **Animation Pack**: UE4 Animation Starter Pack
- **FPS Weapon Bundle**: Third-party asset pack

## License

This project is an educational/portfolio piece. Please refer to individual asset licenses for usage rights.

## Contact

**Repository**: [github.com/Overproness/zombie_game_DSA_Project](https://github.com/Overproness/zombie_game_DSA_Project)

---

**Note**: This is a Data Structures and Algorithms (DSA) project demonstrating game development concepts including AI behavior trees, state management, and cooperative networking systems.
