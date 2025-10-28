import * as React from "react";
import Grid from "@mui/material/Grid";
import Box from "@mui/material/Box";
import Typography from "@mui/material/Typography";
import StatCard from "../dashboard/dashboard/components/StatCard";
import CircularWithValueLabel from "../dashboard/dashboard/components/Circularpercent";
import GaugePointer from "../dashboard/dashboard/components/VelocimeterRpm";
import DynamicLineChart from "../dashboard/dashboard/components/RealTimechart";
import { useSimulatedPID } from "../hooks/UseSimulatedPID";

export default function View1() {
  const referenceValue = 50; 
  const liveValue = useSimulatedPID(referenceValue, 500);

  const maxLength = 30;        // longitud máxima de la gráfica
  const updateInterval = 1000; // intervalo en ms

  const [data, setData] = React.useState([]);
  const [timeLabels, setTimeLabels] = React.useState([]);
  const [timeCounter, setTimeCounter] = React.useState(0); // tiempo absoluto
  const [trend, setTrend] = React.useState("neutral");
  const [trendLabel, setTrendLabel] = React.useState("+0%");

  React.useEffect(() => {
    const interval = setInterval(() => {
      setData(prevData => {
        const nextValue = Math.floor(Math.random() * (1000 - 200 + 1)) + 200;

        // actualizar datos
        const newData = [...prevData, nextValue].slice(-maxLength);

        // calcular tendencia
        if (prevData.length > 0) {
          const diff = nextValue - prevData[prevData.length - 1];
          const percentChange = (diff / prevData[prevData.length - 1]) * 100;
          const newTrend = percentChange > 2 ? "up" : percentChange < -2 ? "down" : "neutral";
          setTrend(newTrend);
          setTrendLabel(`${diff >= 0 ? "+" : ""}${percentChange.toFixed(1)}%`);
        }

        return newData;
      });

      // actualizar etiquetas de tiempo absoluto
      setTimeLabels(prev => {
        const newLabels = [...prev, `${timeCounter}s`].slice(-maxLength);
        return newLabels;
      });

      setTimeCounter(prev => prev + updateInterval / 1000);
    }, updateInterval);

    return () => clearInterval(interval);
  }, [timeCounter]);

  const cardData = {
    title: "SNR",
    value: "dB",
    interval: "RealTime",
    trend,
    data,
    trendLabel,
    timeLabels,
  };

  return (
    <Box sx={{ width: "100%", maxWidth: { sm: "100%", md: "1700px" } }}>
      <Typography component="h2" variant="h6" sx={{ mb: 2 }}>Overview</Typography>
      <Grid container spacing={2} columns={12} sx={{ mb: (theme) => theme.spacing(2) }}>
        <Grid size={{ xs: 12, sm: 6, lg: 6 }}>
          <CircularWithValueLabel porcentajetolva={70} text="Tolva" />
        </Grid>

        <Grid size={{ xs: 12, sm: 6, lg: 6 }}>
          <StatCard {...cardData} />
        </Grid>

        <Grid size={{ xs: 12, sm: 6, lg: 6 }}>
          <GaugePointer max={350} rpm={270} />
        </Grid>

        <Grid size={{ xs: 12, sm: 6, lg: 6 }}>
          <DynamicLineChart liveValue={liveValue} referenceValue={referenceValue} title="Señal del Control PID" />
        </Grid>
      </Grid>
    </Box>
  );
}
