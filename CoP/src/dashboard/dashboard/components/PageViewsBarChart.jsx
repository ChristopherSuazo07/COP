import * as React from 'react';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import Chip from '@mui/material/Chip';
import Typography from '@mui/material/Typography';
import Stack from '@mui/material/Stack';
import { BarChart } from '@mui/x-charts/BarChart';
import { useTheme } from '@mui/material/styles';

export default function PageViewsBarChart({ dataArray }) {
  const theme = useTheme();
  const colorPalette = [
    (theme.vars || theme).palette.primary.dark,
    (theme.vars || theme).palette.primary.main,
    (theme.vars || theme).palette.primary.light,
  ];

  const datasemanal =[];
  for(let i = 0; i < 7; i++){
    datasemanal.push(dataArray[i]);  
  }
  console.log(datasemanal)
  return (
    <Card variant="outlined" sx={{ width: '100%' }}>
      <CardContent>
        <Typography component="h2" variant="subtitle1" gutterBottom>
          Producción Semanal
        </Typography>
        <Stack sx={{ justifyContent: 'space-between' }}>
          <Stack
            direction="row"
            sx={{
              alignContent: { xs: 'center', sm: 'flex-start' },
              alignItems: 'center',
              gap: 1,
            }}
          >
            
          </Stack>
          <Typography variant="caption" sx={{ color: 'text.secondary' }}>
            Bolsas de Frijol  dispensadas en última semana
          </Typography>
        </Stack>

        <BarChart
          borderRadius={100}
          colors={colorPalette}
          xAxis={[
            {
              scaleType: 'band',
              categoryGapRatio: 0.3,
              data: ['Lunes', 'Martes', 'Miércoles', 'Jueves', 'Viernes', 'Sábado', 'Domingo'],
              height: 24,
            },
          ]}
          yAxis={[{ width: 40 }]}
          series={[
            {
              id: 'page-views',
              label: 'Bolsas Dispensadas',
              data: datasemanal,
              stack: 'A',
            },
           
          ]}
          height={250}
          margin={{ left: 0, right: 0, top: 20, bottom: 0 }}
          grid={{ horizontal: true }}
          hideLegend
        />
      </CardContent>
    </Card>
  );
}
