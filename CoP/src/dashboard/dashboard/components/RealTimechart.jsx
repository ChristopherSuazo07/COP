import React, { useEffect, useRef, useState } from "react";
import CanvasJSReact from "@canvasjs/react-charts";
import Card from "@mui/material/Card";
import Typography from "@mui/material/Typography";

const CanvasJSChart = CanvasJSReact.CanvasJSChart;


export default function DynamicLineChart({
  liveValue,
  referenceValue,
  title = "Control PID",
}) {
  const chartRef = useRef(null);
  const [dataPoints, setDataPoints] = useState([]);
  const xVal = useRef(1);

  // 🔹 Cada vez que cambia el valor recibido del WebSocket, se actualiza el gráfico
  useEffect(() => {
    if (liveValue === undefined || liveValue === null) return;

    setDataPoints((prev) => {
      const newPoints = [...prev, { x: xVal.current, y: liveValue }];
      xVal.current++;
      if (newPoints.length > 10) newPoints.shift(); // mantener 10 puntos visibles
      return newPoints;
    });

    if (chartRef.current) chartRef.current.render();
  }, [liveValue]);

  const options = {
    backgroundColor: "transparent",
    title: {
      text: "",
      fontColor: "white",
    },
    axisY: {
      title: "Valor",
      titleFontColor: "white",
      labelFontColor: "white",
      gridColor: "rgba(255, 255, 255, 0.2)",
      lineColor: "rgba(255, 255, 255, 0.3)",
    },
    axisX: {
      title: "Tiempo (s)",
      titleFontColor: "white",
      labelFontColor: "white",
      gridColor: "rgba(255, 255, 255, 0.2)",
      lineColor: "rgba(255, 255, 255, 0.3)",
    },
    data: [
      {
        type: "line",
        dataPoints: dataPoints,
        lineColor: "rgba(0, 110, 154, 1)",
        markerColor: "#02134eff",
        lineThickness: 2,
      },
      {
        type: "line",
        lineDashType: "dash",
        color: "#ff4081",
        lineThickness: 2,
        dataPoints: dataPoints.map((point) => ({
          x: point.x,
          y: referenceValue,
        })),
      },
    ],
  };

  return (
    <Card
      variant="outlined"
      sx={{
        width: "100%",
        display: "flex",
        flexDirection: "column",
        justifyContent: "center",
        alignItems: "center",
        p: 2,
        backgroundColor: "transparent",
      }}
    >
      <Typography component="h2" variant="subtitle1" gutterBottom>
        {title}
      </Typography>
      <CanvasJSChart
        options={options}
        onRef={(ref) => (chartRef.current = ref)}
      />
    </Card>
  );
}
