# Caddy AI2 ROS2 Teleop Radio ESP32 Bridge

[![ROS 2](https://img.shields.io/badge/ROS%202-Jazzy-blue)](https://docs.ros.org/en/jazzy/index.html)
[![Platform](https://img.shields.io/badge/Platform-ESP32-green)](https://www.espressif.com/en/products/socs/esp32)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## 📋 Descripción

Puente de comunicación entre una radio RC y ROS 2 (Jazzy) utilizando micro-ROS y un ESP32 como microcontrolador (MCU). El proyecto está desarrollado con VSCode y PlatformIO.

### Características principales

- ✅ Publica mensajes `sensor_msgs/Joy` en el topic `/joy`
- ✅ 6 ejes (canales) de la radio RC
- ✅ Detección de estado de conexión mediante el estado de un botón
- ✅ Frecuencia de operación: 50 Hz
- ✅ Soporte para teleoperación Twist y Ackermann
- ✅ Lanzamiento automático del agente micro-ROS mediante Docker

## 🎮 Mapeo de Canales

![Canales de la radio](img/canales.jpg)

| Canal | Eje | Función típica | Rango |
|-------|-----|----------------|-------|
| CH1   | 0   | Dirección      | -1.0 a 1.0 |
| CH2   | 1   | Aceleración    | -1.0 a 1.0 |
| CH3   | 2   | Throttle       | -1.0 a 1.0 |
| CH4   | 3   | Yaw            | -1.0 a 1.0 |
| CH5   | 4   | Auxiliar 1     | -1.0 a 1.0 |
| CH6   | 5   | Auxiliar 2     | -1.0 a 1.0 |

> ⚠️ **Nota**: Cuando se apaga el canal 3, automáticamente baja a 0.


| Botón | Función típica         | Valores    |
|-------|------------------------|------------|
| 0     | Enlace radio conectado | {0.0, 1.0} |
| 1     | CH5 > 1500, modo turbo | {0.0, 1.0} |

## 🖥️ Requisitos del Sistema

- **OS**: Ubuntu 24.04 LTS (o compatible)
- **ROS 2**: Jazzy Jalisco
- **Hardware**: ESP32 + Radio RC de 6 canales
- **Software**: Docker, PlatformIO (desarrollo)

## 📦 Instalación

### 1. Instalar Docker

Sigue la [guía oficial de Docker](https://docs.docker.com/engine/install/ubuntu/#install-using-the-repository) y asigna permisos al usuario:

```bash
sudo usermod -aG docker $USER
```

> 🔄 **Importante**: Cierra sesión y vuelve a iniciarla para aplicar los cambios.

### 2. Instalar dependencias de ROS 2

```bash
sudo apt-get update
sudo apt-get install ros-jazzy-teleop-twist-joy
```

### 3. Configurar permisos del puerto serie

```bash
sudo usermod -a -G dialout $USER
```

> 🔄 Reinicia la sesión para aplicar los cambios.

### 4. Clonar y compilar el paquete

```bash
cd ~/ros2_ws/src
git clone <URL_DEL_REPOSITORIO> ros2_caddy_ai2_joystick_esp32_radio_bridge
cd ~/ros2_ws
colcon build --packages-select ros2_caddy_ai2_joystick_esp32_radio_bridge
source install/setup.bash
```

### 5. Flashear el ESP32

1. Abre el proyecto en VSCode con PlatformIO
2. Navega a `Platformio/joy/`
3. Conecta el ESP32 vía USB
4. Compila y sube el código:
   ```bash
   pio run --target upload
   ```

## 🚀 Uso

### Identificar el puerto USB del ESP32

```bash
sudo dmesg | grep tty
```

Busca algo como `/dev/ttyUSB0` o `/dev/ttyACM0`.

### Opción 1: Lanzar solo el puente micro-ROS

```bash
ros2 launch caddy_ai2_ros2_teleop_radio_esp32_bridge joy_bridge.launch.py
```

Con puerto personalizado:
```bash
ros2 launch caddy_ai2_ros2_teleop_radio_esp32_bridge joy_bridge.launch.py usb_device:=/dev/ttyUSB1
```

### Opción 2: Lanzar con teleoperación Twist

```bash
ros2 launch caddy_ai2_ros2_teleop_radio_esp32_bridge joy_teleop_twist.launch.py
```

### Verificar funcionamiento

```bash
# Ver mensajes del joystick
ros2 topic echo /joy

# Ver comandos de velocidad
ros2 topic echo /bicycle_steering_controller/reference
```

### Modo manual (sin launch files)

Si prefieres ejecutar el agente micro-ROS manualmente:

```bash
docker run -it --rm -v /dev:/dev -v /dev/shm:/dev/shm --privileged --net=host microros/micro-ros-agent:$ROS_DISTRO serial --dev /dev/ttyUSB0
```

## 📁 Estructura del Proyecto

```
ros2_caddy_ai2_joystick_esp32_radio_bridge/
├── bringup/
│   ├── config/                    # Archivos de configuración
│   │   └── joy_teleop_twist.yaml
│   └── launch/                    # Launch files
│       ├── joy_bridge.launch.py
│       └── joy_teleop_twist.launch.py
├── Platformio/
│   └── joy/                       # Código del ESP32
│       ├── lib/
│       │   └── PWMReaderLibrary/  # Librería para leer PWM
│       ├── src/
│       │   └── main.cpp           # Código principal
│       └── platformio.ini
├── img/                           # Imágenes del proyecto
└── README.md
```

## 🔧 Configuración

### Ajustar mapeo de ejes

Edita los archivos en `bringup/config/`:

**Para Twist** (`joy_teleop_twist.yaml`):
```yaml
teleop_twist_joy_node:
  ros__parameters:
  
    # ————— Botones de activación —————
    require_enable_button: true      # Si requiere botón de habilitación para moverse
    enable_button: 0                 # Índice de botón para movimiento normal
    enable_turbo_button: 1          # Índice de botón para turbo (desactivado si -1)

    # ————— Ejes lineales —————
    axis_linear:
      x: 1                          # Eje del joystick para avance/retroceso

    # Escalas de velocidad lineal
    scale_linear:
      x: 0.7

    # Escalas de velocidad lineal en turbo
    scale_linear_turbo:
      x: 1.0

    # ————— Ejes angulares —————
    axis_angular:
      yaw: 0                        # Eje para rotación Yaw (giro)

    # Escalas de velocidad angular
    scale_angular:
      yaw: 0.2

    # Escalas de velocidad angular en turbo
    scale_angular_turbo:
      yaw: 0.4

    # ————— Comportamiento extra —————
    inverted_reverse: false          # Invierte giro cuando se va en reversa

    # ————— Publicación de tipo de mensaje —————
    publish_stamped_twist: true      # Publica `TwistStamped` en vez de `Twist`
    frame: ""               # `frame_id` usado en el header de TwistStamped
```

### Diagrama de flujo

```
Radio RC → ESP32 → /joy → teleop_twist_joy_node → /bicycle_steering_controller/reference
```

## 🐛 Solución de Problemas

### El ESP32 no se conecta

1. Verifica el puerto USB:
   ```bash
   ls -l /dev/ttyUSB* /dev/ttyACM*
   ```

2. Comprueba permisos:
   ```bash
   sudo chmod 666 /dev/ttyUSB0
   ```

(*) Aunque con los permisos de dialout debería ser suficiente

3. Reinicia el ESP32

### No se publican mensajes en /joy

1. Verifica que el agente micro-ROS esté corriendo:
   ```bash
   docker ps
   ```

2. Comprueba la conexión serie en el ESP32:
   ```bash
   pio device monitor
   ```

### Docker no encuentra la imagen

```bash
docker pull microros/micro-ros-agent:jazzy
```

## 📸 Galería

<table>
  <tr>
    <td><img src="img/radio.jpg" alt="Radio RC" width="300"/></td>
    <td><img src="img/caja_radio.jpg" alt="Caja de la radio" width="300"/></td>
    <td><img src="img/esp32.jpg" alt="ESP32" width="300"/></td>
  </tr>
  <tr>
    <td align="center">Radio RC</td>
    <td align="center">Receptor en caja</td>
    <td align="center">ESP32</td>
  </tr>
</table>

## 📚 Referencias

### Documentación oficial
- [micro-ROS Documentation](https://micro.ros.org/docs/tutorials/advanced/handling_type_memory/)
- [sensor_msgs/Joy Message](https://docs.ros2.org/latest/api/sensor_msgs/msg/Joy.html)
- [std_msgs/Header Message](https://docs.ros2.org/latest/api/std_msgs/msg/Header.html)

### Repositorios relacionados
- [micro_ros_platformio](https://github.com/micro-ROS/micro_ros_platformio)
- [micro-ROS Demos](https://github.com/micro-ROS/micro-ROS-demos/tree/jazzy)
- [micro_ros_arduino](https://github.com/micro-ROS/micro_ros_arduino)
- [micro-ROS Examples](https://github.com/micro-ROS/micro_ros_arduino/tree/jazzy/examples)

## 👥 Autores

- **Desarrollador Principal**: Rafael Carbonell Lázaro (racarla96)
- **Proyecto**: Caddy AI2 - Proyecto CERVAREC

## 📄 Licencia

Copyright (c) 2025, Rafael Carbonell Lázaro (racarla96)

Este proyecto se distribuye bajo la licencia **Creative Commons Attribution 4.0 International (CC BY 4.0)**.

### En resumen:

✅ **Puedes:**
- Usar, modificar y redistribuir la librería
- Utilizarla en proyectos comerciales o privados
- Crear trabajos derivados

⚠️ **Debes:**
- Mantener atribución al autor/proyecto (en documentación, créditos, "About" de la aplicación, etc.)
- Indicar si se realizaron cambios
- Proporcionar un enlace a la licencia

❌ **No puedes:**
- Imponer restricciones adicionales que impidan a otros ejercer los permisos que otorga la licencia

### Texto legal completo:
https://creativecommons.org/licenses/by/4.0/legalcode

### Atribución sugerida:

Este proyecto utiliza " caddy_ai2_ros2_teleop_radio_esp32_bridge"
desarrollado por Rafael Carbonell Lázaro (racarla96)
Licencia: CC BY 4.0 (https://creativecommons.org/licenses/by/4.0/)
