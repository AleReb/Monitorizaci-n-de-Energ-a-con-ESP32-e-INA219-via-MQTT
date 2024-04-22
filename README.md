# Monitorizaci-n-de-Energ-a-con-ESP32-e-INA219-via-MQTT
## Descripción General
Este proyecto utiliza el ESP32 junto con el sensor INA219 para medir corriente y voltaje en tiempo real. Los datos medidos se envían a través de MQTT, lo que permite un monitoreo remoto y en tiempo real de las métricas de energía.

## Componentes y Tecnologías
- **ESP32**: Microcontrolador con capacidades Wi-Fi y Bluetooth.
- **INA219**: Sensor de medición de corriente y voltaje.
- **MQTT**: Protocolo de mensajería ligero ideal para comunicaciones M2M.
- **Librería Wire**: Para la comunicación I2C con el sensor INA219.

## Instalación y Uso
- Configurar el ESP32 y conectar el sensor INA219 siguiendo las definiciones de pines especificadas en los archivos.
- Asegurarse de tener una instancia MQTT corriendo para recibir los datos enviados por el dispositivo.
- Cargar el código en el ESP32 y calibrar el sensor INA219 si es necesario.
- Observar los datos transmitidos a través de MQTT en una plataforma o dashboard compatible.

## Ejemplos de Uso
- Monitoreo del consumo energético de electrodomésticos.
- Implementación en sistemas de gestión de energía para edificios inteligentes.
- Proyectos de IoT para optimización del uso de energía en aplicaciones industriales.

## Contribuciones y Desarrollo Futuro
- Expansión del proyecto para soportar múltiples sensores INA219 y agregar funcionalidades de control de dispositivos basado en los datos de consumo.
- Mejorar la seguridad de las transmisiones MQTT.
- Integración con plataformas avanzadas de análisis de datos y machine learning para predicciones y alertas.
"""
