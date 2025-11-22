import * as React from "react";
import Grid from "@mui/material/Grid";
import Box from "@mui/material/Box";
import Typography from "@mui/material/Typography";
import StatCard from "../dashboard/dashboard/components/StatCard";
import CircularWithValueLabel from "../dashboard/dashboard/components/Circularpercent";
import GaugePointer from "../dashboard/dashboard/components/VelocimeterRpm";
import DynamicLineChart from "../dashboard/dashboard/components/RealTimechart";
import { useSimulatedPID } from "../hooks/UseSimulatedPID";
import { getFirebaseDB } from "../config/firebaseConfig";
import { ref, onValue } from "firebase/database";

export default function View1() {
  const referenceValue = 50; 
  const liveValue = useSimulatedPID(referenceValue, 500);

  // Estados para valores en tiempo real
  const [tolva, setTolva] = React.useState(0);
  const [snr, setSnr] = React.useState(0);
  const [rpm, setRpm] = React.useState(0);

  // Gráfica en tiempo real (igual que antes)
  const maxLength = 30;
  const updateInterval = 1000;
  const [data, setData] = React.useState([]);
  const [timeLabels, setTimeLabels] = React.useState([]);
  const [timeCounter, setTimeCounter] = React.useState(0);
  const [trend, setTrend] = React.useState("up");
  const [trendLabel, setTrendLabel] = React.useState("+0%");

  // ==================== Firebase en tiempo real ====================
  React.useEffect(() => {
    const db = getFirebaseDB();
    const parametrosRef = ref(db, "Parametros");

    const unsub = onValue(parametrosRef, (snapshot) => {
      const data = snapshot.val();
      if (!data) return;

      setTolva(data.Tolva ?? 0);
      setSnr(data.SNR ?? 0);
      setRpm(data.RPM ?? 0);
    });

    return () => unsub();
  }, []);

  // ==================== Gráfica de SNR ====================
  React.useEffect(() => {
    const interval = setInterval(() => {
      setData(prevData => {
        const nextValue = snr; // ahora la SNR real viene de FB
        const newData = [...prevData, nextValue].slice(-maxLength);

        if (prevData.length > 0) {
          const diff = nextValue - prevData[prevData.length - 1];
          const percentChange = (diff / prevData[prevData.length - 1]) * 100;
          const newTrend = percentChange > 2 ? "up" : percentChange < -2 ? "down" : "up";
          setTrend(newTrend);
          setTrendLabel(`${diff >= 0 ? "+" : ""}${percentChange.toFixed(1)}%`);
        }

        return newData;
      });

      setTimeLabels(prev => {
        const newLabels = [...prev, `${timeCounter}s`].slice(-maxLength);
        return newLabels;
      });

      setTimeCounter(prev => prev + updateInterval / 1000);
    }, updateInterval);

    return () => clearInterval(interval);
  }, [timeCounter, snr]);

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
          <CircularWithValueLabel porcentajetolva={tolva} text="Tolva" />
        </Grid>

        <Grid size={{ xs: 12, sm: 6, lg: 6 }}>
          <StatCard {...cardData} />
        </Grid>

        <Grid size={{ xs: 12, sm: 6, lg: 6 }}>
          <GaugePointer max={350} rpm={rpm} />
        </Grid>

        
      </Grid>
    </Box>
  );
}
