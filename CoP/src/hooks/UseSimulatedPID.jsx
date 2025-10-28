import { useState, useEffect, useRef } from "react";

/**
 * Simula un sistema tipo PID con respuesta suave
 * setpoint: valor objetivo
 * interval: intervalo de actualización (ms)
 * Kp: ganancia proporcional
 * Kd: factor de “derivada” para suavizar cambios
 */
export function useSimulatedPID(setpoint = 70, interval = 500) {
  const [value, setValue] = useState(setpoint);
  const pv = useRef(setpoint);       // valor actual del sistema
  const velocity = useRef(0);        // velocidad simulada
  const Kp = 0.3;                    // control proporcional
  const Kd = 0.5;                    // amortiguamiento (derivada)
  
  useEffect(() => {
    const id = setInterval(() => {
      const error = setpoint - pv.current;

      // Aceleración proporcional al error + amortiguamiento
      let accel = Kp * error - Kd * velocity.current;

      // Añadir un pequeño ruido aleatorio para simular sensor real
      accel += Math.random() * 2 - 1;

      // Actualizar velocidad y posición
      velocity.current += accel;
      pv.current += velocity.current;

      setValue(pv.current);
    }, interval);

    return () => clearInterval(id);
  }, [setpoint, interval]);

  return value;
}
