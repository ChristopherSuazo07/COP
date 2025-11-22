// src/components/CustomizedDataGrid.jsx
import * as React from "react";
import { DataGrid } from "@mui/x-data-grid";
import Chip from "@mui/material/Chip";
import AccessTimeIcon from "@mui/icons-material/AccessTime";
import { ref, onValue } from "firebase/database";
import { getFirebaseDB } from "../../../config/firebaseConfig";

// ==================== Helpers ====================
function renderStatus(status) {
  const colors = { Exitoso: "success", Incompleto: "default" };
  return <Chip label={status} color={colors[status]} size="small" />;
}

function renderHora(hora) {
  return (
    <div style={{ display: "flex", alignItems: "center" }}>
      <AccessTimeIcon style={{ fontSize: "16px", marginRight: 4 }} />
      <span>{hora}</span>
    </div>
  );
}

// ==================== Columns ====================
const columns = [
  { field: "fecha", headerName: "Fecha", flex: 1, minWidth: 120 },
  { field: "status", headerName: "Status", flex: 0.5, minWidth: 100, renderCell: (params) => renderStatus(params.value) },
  { field: "cantidad", headerName: "Cantidad", flex: 0.5, minWidth: 80 },
  { field: "precio", headerName: "Precio", flex: 0.5, minWidth: 100 },
  { field: "hora", headerName: "Hora", flex: 0.7, minWidth: 100, renderCell: (params) => renderHora(params.value) },
];

// ==================== Componente ====================
export default function CustomizedDataGrid() {
  const [rows, setRows] = React.useState([]);

  React.useEffect(() => {
    const db = getFirebaseDB();
    const registrosRef = ref(db, "registrosDiarios");

    const unsub = onValue(registrosRef, (snapshot) => {
      const data = snapshot.val();
      if (!data) return;

      const filas = [];
      Object.keys(data)
        .sort((a, b) => new Date(b) - new Date(a)) // Orden descendente por fecha
        .forEach((fecha) => {
          const horas = data[fecha];
          Object.keys(horas)
            .sort()
            .forEach((h) => {
              const registro = horas[h];
              filas.push({
                id: filas.length + 1,
                fecha,
                status: registro.Exitoso ? "Exitoso" : "Incompleto",
                cantidad: registro.Cantidad ?? 0,
                precio: registro.PrecioVenta ?? 0,
                hora: h, // hora exacta
              });
            });
        });

      setRows(filas);
    });

    return () => unsub();
  }, []);

  return (
    <div style={{ width: "100%", height: "600px" }}>
      <DataGrid
        rows={rows}
        columns={columns}
        checkboxSelection
        getRowClassName={(params) =>
          params.indexRelativeToCurrentPage % 2 === 0 ? "even" : "odd"
        }
        pageSize={20}
        rowsPerPageOptions={[10, 20, 50]}
        disableSelectionOnClick
        autoHeight
      />
    </div>
  );
}
