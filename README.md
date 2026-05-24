stm32_bootloader_with_application/
├── BootLoader/          # Bootloader project (flashed at 0x08000000)
│   └── Core/
│       ├── Src/
│       │   ├── main.c           # Bootloader logic
│       │   ├── usart.c          # UART initialization
│       │   ├── gpio.c           # GPIO initialization
│       │   └── system_stm32f4xx.c
│       └── Inc/
│           ├── main.h
│           ├── usart.h
│           └── gpio.h
├── Application/         # Application project (flashed at 0x08004000)
│   └── Core/
│       ├── Src/
│       │   ├── main.c           # Application logic
│       │   ├── usart.c
│       │   ├── gpio.c
│       │   └── system_stm32f4xx.c
│       └── Inc/
└── README.md

**🛠️ Hardware Required**
Component       Details 
Board           STM32F4Discovery
Chip            STM32F407VGTx
IDE             STM32CubeIDE
Flash Tool      STM32CubeProgrammer
Cable           Mini-USB (CN1 port)

**🗺️ Flash Memory Layout**
Flash Memory (1MB total):
┌─────────────────────────────┐ 0x08000000
│   BOOTLOADER (Sector 0)     │ 16KB
├─────────────────────────────┤ 0x08004000
│   APPLICATION (Sector 1+)   │ 1008KB
└─────────────────────────────┘ 0x080FFFFF

**⚙️ How It Works**
Boot Sequence
Power ON / RESET
      ↓
Bootloader starts at 0x08000000
      ↓
Hardware Init (HAL, Clock, GPIO, UART)
      ↓
Is USER button (PA0) held?
      ├── YES → Stay in bootloader mode → 🟢 Green LED blinks
      └── NO  → Check for valid app
                      ↓
              Is app valid at 0x08004000?
              ├── NO  → 🔴 Red LED blinks (no app found)
              └── YES → 🔵 Blue LED ON (1 sec) → Jump to App
                                ↓
                        Application runs
                        🔵 Blue LED blinks continuously
I have used the built-in functions of STM32CubeIDE for this project.
