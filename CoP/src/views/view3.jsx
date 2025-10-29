import * as React from "react";
import Grid from "@mui/material/Grid";
import Box from "@mui/material/Box";
import Typography from "@mui/material/Typography";
import CustomizedDataGrid from "../dashboard/dashboard/components/CustomizedDataGrid";
import PageViewsBarChart from "../dashboard/dashboard/components/PageViewsBarChart";
import Ventahora from "../dashboard/dashboard/components/VentaporHora";
import CustomizedTreeView from "../dashboard/dashboard/components/CustomizedTreeView";

const hourlyData = [
  12, 15, 18, 20, 22, 25, 30, 28, 26, 24, 22, 20, 18, 16, 15, 14, 12, 13, 14,
  15, 16, 18, 20, 22,
];

export default function View2() {
  return (
    <Box sx={{ width: "100%", maxWidth: { sm: "100%", md: "1700px" } }}>
      <Grid
        container
        spacing={2}
        columns={12}
        sx={{ mb: (theme) => theme.spacing(2) }}
      >
        <Grid size={{ xs: 12, sm: 6, lg: 6 }}>
          <Ventahora
            title="Ventas por Hora"
            value={10} // punto de equilibrio
            interval="Últimas 24 horas"
            data={hourlyData}
          />
        </Grid>
        <Grid size={{ xs: 12, sm: 6, lg: 6 }}>
          <PageViewsBarChart dataArray={[50, 15, 20, 40, 35, 60, 70]} />
        </Grid>

        <Grid size={{ xs: 12, sm: 6, lg: 9 }}>
          <CustomizedDataGrid />
        </Grid>
         <Grid size={{ xs: 12, sm: 6, lg: 3 }}>
          <CustomizedTreeView />
        </Grid>
      </Grid>
    </Box>
  );
}
