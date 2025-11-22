import * as React from "react";
import Grid from "@mui/material/Grid";
import Box from "@mui/material/Box";
import { getFirebaseDB } from "../config/firebaseConfig";
import { ref, onValue } from "firebase/database";

import CustomizedDataGrid from "../dashboard/dashboard/components/CustomizedDataGrid";
import PageViewsBarChart from "../dashboard/dashboard/components/PageViewsBarChart";
import Ventahora from "../dashboard/dashboard/components/VentaporHora";
import CustomizedTreeView from "../dashboard/dashboard/components/CustomizedTreeView";

export default function View2() {
  const [hourlyData, setHourlyData] = React.useState([]);
  const [weeklyData, setWeeklyData] = React.useState([]);

  const db = getFirebaseDB();

  // -------- 🔹 Obtener la fecha del día en formato YYYY-MM-DD --------
  function getToday() {
    const today = new Date();
    return today.toISOString().split("T")[0];
  }

  // -------- 🔹 Obtener número de semana ISO (semana_YYYY-WW) --------
  function getISOWeekString() {
    const date = new Date();

    // Copia para no alterar original
    const temp = new Date(Date.UTC(date.getFullYear(), date.getMonth(), date.getDate()));
    const dayNum = temp.getUTCDay() || 7;
    temp.setUTCDate(temp.getUTCDate() + 4 - dayNum);

    const yearStart = new Date(Date.UTC(temp.getUTCFullYear(), 0, 1));
    const weekNo = Math.ceil(((temp - yearStart) / 86400000 + 1) / 7);

    return `semana_${temp.getUTCFullYear()}-${String(weekNo).padStart(2, "0")}`;
  }

  // -------- 🔹 Cargar ventas por hora --------
  React.useEffect(() => {
    const today = getToday();
    const path = `ventasPorHora/${today}`;
    const nodeRef = ref(db, path);

    onValue(nodeRef, (snapshot) => {
      const data = snapshot.val() || {};

      // Asegurar orden correcto → 08:00 a 17:00
      const orderedHours = Object.keys(data)
        .sort()
        .map((h) => data[h]);

      setHourlyData(orderedHours);
    });
  }, []);

  // -------- 🔹 Cargar ventas por semana --------
  React.useEffect(() => {
    const weekString = getISOWeekString();
    const path = `ventasPorSemana/${weekString}`;
    const nodeRef = ref(db, path);

    onValue(nodeRef, (snapshot) => {
      const data = snapshot.val() || {};

      // Orden correcto: lun, mar, mier, juev, vier, sab, dom
      const order = ["lunes", "martes", "miercoles", "jueves", "viernes", "sabado", "domingo"];

      const orderedDays = order.map((day) => data[day] || 0);

      setWeeklyData(orderedDays);
    });
  }, []);

  return (
    <Box sx={{ width: "100%", maxWidth: { sm: "100%", md: "1700px" } }}>
      <Grid container spacing={2} columns={12} sx={{ mb: 2 }}>
        
        {/* ---- Ventas por hora ---- */}
        <Grid size={{ xs: 12, sm: 6, lg: 6 }}>
          <Ventahora
            title="Ventas por Hora"
            value={hourlyData.reduce((a, b) => a + b, 0)} // total del día
            interval="Hoy"
            data={hourlyData}
          />
        </Grid>

        {/* ---- Ventas por semana ---- */}
        <Grid size={{ xs: 12, sm: 6, lg: 6 }}>
          <PageViewsBarChart
            dataArray={weeklyData} // ahora dinámico desde Firebase
          />
        </Grid>

        <Grid size={{ xs: 12, sm: 12, lg: 12 }}>
          <CustomizedDataGrid />
        </Grid>
{/* 
        <Grid size={{ xs: 12, sm: 6, lg: 3 }}>
          <CustomizedTreeView />
        </Grid> */}

      </Grid>
    </Box>
  );
}
