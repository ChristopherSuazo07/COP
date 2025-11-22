// src/components/HistorialTabla.jsx
import * as React from "react";
import Avatar from "@mui/material/Avatar";
import Chip from "@mui/material/Chip";
import { DataGrid } from "@mui/x-data-grid";
import { SparkLineChart } from "@mui/x-charts/SparkLineChart";
import { ref, onValue } from "firebase/database";
import { getFirebaseDB } from "../../../../config/firebaseConfig";

// ==================== Helpers ====================
function renderSparklineCell(params) {
  const { value, colDef } = params;
  if (!value || value.length === 0) return null;

  const dataHoras = Array.from({ length: value.length }, (_, i) =>
    i.toString().padStart(2, "0") + ":00"
  );

  return (
    <div style={{ display: "flex", alignItems: "center", height: "100%" }}>
      <SparkLineChart
        data={value}
        width={colDef.computedWidth || 100}
        height={32}
        plotType="bar"
        showHighlight
        showTooltip
        color="hsl(210, 98%, 42%)"
        xAxis={{ scaleType: "band", data: dataHoras }}
      />
    </div>
  );
}

function renderStatus(status) {
  const colors = { Exitoso: "success", Incompleto: "default" };
  return <Chip label={status} color={colors[status]} size="small" />;
}

export function renderAvatar(params) {
  if (!params.value) return null;
  return (
    <Avatar
      sx={{ backgroundColor: params.value.color, width: 24, height: 24, fontSize: "0.85rem" }}
    >
      {params.value.name.toUpperCase().substring(0, 1)}
    </Avatar>
  );
}

// ==================== Columns ====================
export const columns = [
  {
    field: "pageTitle",
    headerName: "Fecha de Dispensado",
    flex: 1.5,
    minWidth: 200,
  },
  {
    field: "status",
    headerName: "Status",
    flex: 0.5,
    minWidth: 105,
    renderCell: (params) => renderStatus(params.value),
  },
  {
    field: "users",
    headerName: "Cantidad",
    headerAlign: "right",
    align: "right",
    flex: 1,
    minWidth: 80,
  },
  {
    field: "eventCount",
    headerName: "Precio de Venta",
    headerAlign: "right",
    align: "right",
    flex: 1,
    minWidth: 120,
  },
  {
    field: "conversions",
    headerName: "Hora",
    headerAlign: "right",
    flex: 1,
    minWidth: 30,
    renderCell: renderSparklineCell,
  },
];

// ==================== Componente ====================
export default function HistorialTabla() {
  const [rows, setRows] = React.useState([]);

  React.useEffect(() => {
    const db = getFirebaseDB();
    const registrosRef = ref(db, "registrosDiarios");

    const unsub = onValue(registrosRef, (snapshot) => {
      const data = snapshot.val();
      if (!data) return;

      const filas = Object.keys(data)
        .sort((a, b) => new Date(b) - new Date(a))
        .map((fecha, index) => {
          const horas = data[fecha];
          const horasOrdenadas = Object.keys(horas).sort();

          const conversions = horasOrdenadas.map((h) => horas[h]?.cantidad ?? 0);
          const totalCantidad = conversions.reduce((a, b) => a + b, 0);
          const totalPrecio = horasOrdenadas.reduce(
            (sum, h) => sum + (horas[h]?.precioVenta ?? 0),
            0
          );

          return {
            id: index + 1,
            pageTitle: fecha,
            status: horasOrdenadas.length === 24 ? "Exitoso" : "Incompleto",
            users: totalCantidad,
            eventCount: totalPrecio,
            conversions,
          };
        });

      setRows(filas);
    });

    return () => unsub();
  }, []);

  return (
    <div style={{ width: "100%", height: "600px" }}>
      <DataGrid
        rows={rows}
        columns={columns} // ahora importable externamente
        pageSize={10}
        rowsPerPageOptions={[10, 20, 50]}
        autoHeight
        disableSelectionOnClick
      />
    </div>
  );
}
