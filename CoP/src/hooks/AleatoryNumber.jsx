// src/hooks/useNumeroAleatorio.js
import { useState, useEffect } from "react";

export function useArrayAleatorio(length = 30, intervalo = 1000, max = 2000) {
  const [data, setData] = useState(
    Array.from({ length }, () => Math.floor(Math.random() * max))
  );

  useEffect(() => {
    const id = setInterval(() => {
      setData((prev) => {
        const nuevo = [...prev.slice(1), Math.floor(Math.random() * max)];
        return nuevo;
      });
    }, intervalo);

    return () => clearInterval(id);
  }, [intervalo, max, length]);

  return data;
}
