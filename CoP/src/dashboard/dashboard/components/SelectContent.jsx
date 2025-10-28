import * as React from 'react';
import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';
import { styled } from '@mui/material/styles';
import CheckCircleRoundedIcon from '@mui/icons-material/CheckCircleRounded';
import HighlightOffRoundedIcon from '@mui/icons-material/HighlightOffRounded';

const StatusContainer = styled(Box)(({ theme, working }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: theme.spacing(1),
  width: 215,
  maxHeight: 56,
  padding: theme.spacing(1.5),
  borderRadius: theme.shape.borderRadius,
  border: `2px solid ${working ? theme.palette.success.main : theme.palette.error.main}`,
  boxShadow: `0 0 8px ${working ? theme.palette.success.light : theme.palette.error.light}`,
  backgroundColor: (theme.vars || theme).palette.background.paper,
}));

const StatusIcon = styled('div')(({ working, theme }) => ({
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'center',
  color: working ? theme.palette.success.main : theme.palette.error.main,
}));

export default function SystemStatus({ working }) {
  return (
    <StatusContainer working={working}>
      <StatusIcon working={working}>
        {working ? (
          <CheckCircleRoundedIcon fontSize="medium" />
        ) : (
          <HighlightOffRoundedIcon fontSize="medium" />
        )}
      </StatusIcon>
      <Typography variant="body1">
        {working ? 'Sistema Trabajando' : 'Sistema Detenido'}
      </Typography>
    </StatusContainer>
  );
}
