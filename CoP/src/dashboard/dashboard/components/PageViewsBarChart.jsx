import * as React from 'react';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import Typography from '@mui/material/Typography';
import Stack from '@mui/material/Stack';
import { Bar } from 'react-chartjs-2';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  BarElement,
  Title,
  Tooltip,
  Legend,
} from 'chart.js';

ChartJS.register(CategoryScale, LinearScale, BarElement, Title, Tooltip, Legend);

export default function PageViewsBarChart({ dataArray }) {
  // Limita a 7 datos (una semana)
  const datasemanal = dataArray.slice(0, 7);

  const labels = ['Lunes', 'Martes', 'Miércoles', 'Jueves', 'Viernes', 'Sábado', 'Domingo'];

  // Genera gradiente vertical para cada barra
  const getGradients = (ctx, chartArea) => {
    const gradients = [];
    datasemanal.forEach(() => {
      const gradient = ctx.createLinearGradient(0, chartArea.bottom, 0, chartArea.top);
      gradient.addColorStop(0, 'rgba(150, 198, 238, 0.8)'); // azul claro abajo
      gradient.addColorStop(1, 'rgba(35, 151, 252, 1)');   // azul vivo arriba
      gradients.push(gradient);
    });
    return gradients;
  };

  const data = {
    labels,
    datasets: [
      {
        label: 'Bolsas Dispensadas',
        data: datasemanal,
        backgroundColor: function(context) {
          const chart = context.chart;
          const { ctx, chartArea } = chart;
          if (!chartArea) return 'rgba(30,136,229,1)'; // fallback
          const gradients = getGradients(ctx, chartArea);
          return gradients[context.dataIndex];
        },
        borderRadius: 4,
      },
    ],
  };

  const options = {
    responsive: true,
    plugins: {
      legend: { display: false },
    },
    scales: {
      x: {
        grid: { display: true, color: 'rgba(255,255,255,0.1)' }, // grid visible
        ticks: { color: '#ffffff' }, // labels blancos
      },
      y: {
        beginAtZero: true,
        grid: { display: true, color: 'rgba(255,255,255,0.1)' }, // grid visible
        ticks: { color: '#ffffff' }, // labels blancos
      },
    },
  };

  return (
    <Card variant="outlined" sx={{ width: '100%' }}>
      <CardContent>
        <Typography component="h2" variant="subtitle1" gutterBottom>
          Producción Semanal
        </Typography>
        <Stack sx={{ justifyContent: 'space-between', mb: 2 }}>
          <Typography variant="caption" sx={{ color: 'text.secondary' }}>
            Bolsas de Frijol dispensadas en la semana
          </Typography>
        </Stack>

        <Bar data={data} options={options} height={250} />
      </CardContent>
    </Card>
  );
}
