import * as React from 'react';
import PropTypes from 'prop-types';
import { useTheme } from '@mui/material/styles';
import Box from '@mui/material/Box';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import Chip from '@mui/material/Chip';
import Stack from '@mui/material/Stack';
import Typography from '@mui/material/Typography';
import { SparkLineChart } from '@mui/x-charts/SparkLineChart';
import { areaElementClasses } from '@mui/x-charts/LineChart';

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

function StatCard({ title, value, interval, data, timeLabels }) {
  const theme = useTheme();

  const chartLabels = timeLabels || data.map((_, i) => `${i}s`);

  // Determinar trend según el último dato comparado con el anterior
  const lastIndex = data.length - 1;
  let trend = 'neutral';
  if (lastIndex > 0) {
    if (data[lastIndex] > data[lastIndex - 1]) trend = 'up';
    else if (data[lastIndex] < data[lastIndex - 1]) trend = 'down';
  }

  const trendColors = {
    up: theme.palette.mode === 'light' ? theme.palette.success.main : theme.palette.success.dark,
    down: theme.palette.mode === 'light' ? theme.palette.error.main : theme.palette.error.dark,
    neutral: theme.palette.mode === 'light' ? theme.palette.grey[400] : theme.palette.grey[700],
  };

  const labelColors = { up: 'success', down: 'error', neutral: 'default' };
  const color = labelColors[trend];
  const chartColor = trendColors[trend];

  // Último dato para mostrar en el chip
  const lastDataLabel = data.length > 0 ? `${data[lastIndex]} dB` : '';

  // ID seguro para el gradiente
  const gradientId = React.useMemo(
    () => `area-gradient-${Math.random().toString(36).substr(2, 9)}`,
    []
  );

  return (
    <Card variant="outlined" sx={{ height: '100%', flexGrow: 1 }}>
      <CardContent>
        <Typography component="h2" variant="subtitle2" gutterBottom sx={{ marginBottom: "5%" }}>
          {title}
        </Typography>
        <Stack direction="column" sx={{ justifyContent: 'space-between', flexGrow: '1', gap: 1 }}>
          <Stack sx={{ justifyContent: 'space-between' }}>
            <Stack direction="row" sx={{ justifyContent: 'space-between', alignItems: 'center' }}>
              <Typography variant="h4" component="p">{value}</Typography>
              <Chip size="small" color={color} label={lastDataLabel} />
            </Stack>
            <Typography variant="caption" sx={{ color: 'text.secondary' }}>{interval}</Typography>
          </Stack>
          <Box sx={{ width: '100%', height: 50 }}>
            <SparkLineChart
              color={chartColor}
              data={data}
              area
              showHighlight
              showTooltip
              xAxis={{ scaleType: 'band', data: chartLabels }}
              sx={{ [`& .${areaElementClasses.root}`]: { fill: `url(#${gradientId})` } }}
            >
              <AreaGradient color={chartColor} id={gradientId} />
            </SparkLineChart>
          </Box>
        </Stack>
      </CardContent>
    </Card>
  );
}

StatCard.propTypes = {
  data: PropTypes.arrayOf(PropTypes.number).isRequired,
  interval: PropTypes.string.isRequired,
  title: PropTypes.string.isRequired,
  value: PropTypes.string.isRequired,
  timeLabels: PropTypes.arrayOf(PropTypes.string),
};

export default StatCard;
