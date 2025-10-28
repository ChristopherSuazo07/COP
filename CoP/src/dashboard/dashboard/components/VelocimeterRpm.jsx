import * as React from "react";
import Card from "@mui/material/Card";
import Box from "@mui/material/Box";
import {
  GaugeContainer,
  GaugeValueArc,
  GaugeReferenceArc,
  useGaugeState,
} from "@mui/x-charts/Gauge";
import Typography from "@mui/material/Typography";

// GaugePointer sigue igual
function GaugePointer() {
  const { valueAngle, outerRadius, cx, cy, value } = useGaugeState();

  if (valueAngle === null) return null;

  const target = {
    x: cx + outerRadius * Math.sin(valueAngle),
    y: cy - outerRadius * Math.cos(valueAngle),
  };

  return (
    <g>
      <circle cx={cx} cy={cy} r={4} fill="red" />
      <path d={`M ${cx} ${cy} L ${target.x} ${target.y}`} stroke="red" strokeWidth={3} />
      <text
        x={cx}
        y={cy + 40} // bajado para que no interfiera con la aguja
        textAnchor="middle"
        fontSize={16}
        fontWeight="bold"
        fill="black"
      >
        {`${Math.round(value)} RPM`}
      </text>
    </g>
  );
}


export default function CompositionExample({ rpm, max }) {
  return (
    <Card
      variant="outlined"
      sx={{
        width: "100%",
        display: "flex",
        flexDirection: "column", // columna para que Typography quede arriba
        justifyContent: "center",
        alignItems: "center",
        p: 2
      }}
    >
      <Typography component="h1" variant="subtitle1" gutterBottom>
        Step Motor RPM
      </Typography>

      <GaugeContainer
        width={300}
        height={400}
        startAngle={-110}
        endAngle={110}
        value={rpm}
        valueMax={max}
        valueMin={0}
      >
        <GaugeReferenceArc />
        <GaugeValueArc />
        <GaugePointer />
      </GaugeContainer>
    </Card>
  );
}
