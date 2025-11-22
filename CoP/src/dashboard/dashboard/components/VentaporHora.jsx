import * as React from "react";
import PropTypes from "prop-types";
import { useTheme } from "@mui/material/styles";
import Box from "@mui/material/Box";
import Card from "@mui/material/Card";
import CardContent from "@mui/material/CardContent";
import Chip from "@mui/material/Chip";
import Stack from "@mui/material/Stack";
import Typography from "@mui/material/Typography";
import { SparkLineChart } from "@mui/x-charts/SparkLineChart";
import { areaElementClasses } from "@mui/x-charts/LineChart";

function AreaGradient({ color, id }) {
  return (
    <defs>
      <linearGradient id={id} x1="50%" y1="0%" x2="50%" y2="100%">
        <stop offset="0%" stopColor={color} stopOpacity={0.3} />
        <stop offset="100%" stopColor={color} stopOpacity={0} />
      </linearGradient>
    </defs>
  );
}

AreaGradient.propTypes = {
  color: PropTypes.string.isRequired,
  id: PropTypes.string.isRequired,
};

function Ventahora({ title, value, interval, data }) {
  const theme = useTheme();

  // Calculamos la suma de los datos
  const total = data.reduce((acc, curr) => acc + curr, 0);

  // Determinamos el trend según la comparación con el punto de equilibrio
  let trend = "neutral";
  if (total < 20) trend = "down";
  else if (total > 20) trend = "up";

  // Colores según trend
  const trendColors = {
    up:
      theme.palette.mode === "light"
        ? theme.palette.success.main
        : theme.palette.success.dark,
    down:
      theme.palette.mode === "light"
        ? theme.palette.error.main
        : theme.palette.error.dark,
    neutral:
      theme.palette.mode === "light"
        ? theme.palette.grey[400]
        : theme.palette.grey[700],
  };
  const labelColors = { up: "success", down: "error", neutral: "default" };
  const color = labelColors[trend];
  const chartColor = trendColors[trend];

  // Labels de 24 horas
  const now = new Date();
  const currentHour = now.getHours();
  const currentDateString = now.toLocaleDateString();
  const chartLabels = Array.from({ length: 24 }).map((_, i) => {
    const hour = (currentHour + i) % 24;
    const period = hour >= 12 ? "PM" : "AM";
    const hour12 = hour % 12 === 0 ? 12 : hour % 12;
    return `${currentDateString} ${hour12} ${period}`;
  });

  // ID seguro para el gradiente
  const gradientId = React.useMemo(
    () => `area-gradient-${Math.random().toString(36).substr(2, 9)}`,
    []
  );

  return (
    <Card
      variant="outlined"
      sx={{
        height: "100%",
        flexGrow: 1,
        borderRadius: 3,
        boxShadow: 2,
        transition: "all 0.3s",
        "&:hover": { boxShadow: 6 },
      }}
    >
      <CardContent sx={{ padding: 1 }}>
        <Typography component="h2" variant="subtitle1" gutterBottom
          sx={{ marginBottom: 6, fontWeight: 600 }}
        >
          {title}
        </Typography>

        <Stack
          direction="column"
          sx={{ justifyContent: "space-between", gap: 1 }}
        >
          {/* Texto fijo en dos líneas + valor */}
          <Stack
            direction="row"
            sx={{ justifyContent: "space-between", alignItems: "center" }}
          >
            <Box
              sx={{
                display: "flex",
                justifyContent: "space-between",
                alignItems: "center",
              }}
            >
              {/* Texto en dos líneas */}
              <Box sx={{ lineHeight: 1.2, marginRight: 2.2 }}>
                <Typography variant="h5" sx={{ fontWeight: 900 }}>
                  Punto de
                </Typography>
                <Typography variant="h5" sx={{ fontWeight: 900 }}>
                  Equilibrio
                </Typography>
              </Box>

              {/* Valor con los dos puntos centrado verticalmente */}
              <Box sx={{ display: "flex", alignItems: "center" }}>
                
                <Typography variant="h3" sx={{ fontWeight: 700 }}>
                  20
                </Typography>
              </Box>
            </Box>
          </Stack>

          {/* Chip con la suma total */}
          <Stack direction="row" sx={{ justifyContent: "flex-end", mt: 0.5 }}>
            <Chip
              size="small"
              color={color}
              label={`${total} sacos`}
              sx={{ fontWeight: 600 }}
            />
          </Stack>

          {/* Intervalo */}
          <Typography variant="caption" sx={{ color: "text.secondary" }}>
            {interval}
          </Typography>

          {/* Gráfico */}
          <Box sx={{ width: "100%", height: 60, marginTop: 1 }}>
            <SparkLineChart
              color={chartColor}
              data={data}
              area
              showHighlight
              showTooltip
              xAxis={{ scaleType: "band", data: chartLabels }}
              sx={{
                [`& .${areaElementClasses.root}`]: {
                  fill: `url(#${gradientId})`,
                },
              }}
            >
              <AreaGradient color={chartColor} id={gradientId} />
            </SparkLineChart>
          </Box>
        </Stack>
      </CardContent>
    </Card>
  );
}

Ventahora.propTypes = {
  data: PropTypes.arrayOf(PropTypes.number).isRequired, // 24 datos
  interval: PropTypes.string.isRequired,
  title: PropTypes.string.isRequired,
  value: PropTypes.number.isRequired, // punto de equilibrio
};

export default Ventahora;
