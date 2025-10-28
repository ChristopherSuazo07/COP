import * as React from 'react';
import List from '@mui/material/List';
import ListItem from '@mui/material/ListItem';
import ListItemButton from '@mui/material/ListItemButton';
import ListItemIcon from '@mui/material/ListItemIcon';
import ListItemText from '@mui/material/ListItemText';
import Stack from '@mui/material/Stack';
import Typography from '@mui/material/Typography';
import HomeRoundedIcon from '@mui/icons-material/HomeRounded';
import AnalyticsRoundedIcon from '@mui/icons-material/AnalyticsRounded';
import PeopleRoundedIcon from '@mui/icons-material/PeopleRounded';
import SettingsRoundedIcon from '@mui/icons-material/SettingsRounded';
import InfoRoundedIcon from '@mui/icons-material/InfoRounded';
import HelpRoundedIcon from '@mui/icons-material/HelpRounded';

const mainListItems = [
  { text: 'Monitoreo', icon: <HomeRoundedIcon /> },
  { text: 'Ajuste de Variables', icon: <AnalyticsRoundedIcon /> },
  { text: 'Historial', icon: <PeopleRoundedIcon /> },
];

const secondaryListItems = [
  { text: 'Settings', icon: <SettingsRoundedIcon /> },
  { text: 'About', icon: <InfoRoundedIcon /> },
  { text: 'Feedback', icon: <HelpRoundedIcon /> },
];

export default function MenuContent() {
  return (
    <Stack sx={{ flexGrow: 1, p: 1, justifyContent: 'space-between' }}>
      {/* Main list */}
      <List dense sx={{ gap: 1.5 }}>
        {mainListItems.map((item, index) => (
          <ListItem key={index} disablePadding sx={{ display: 'block' }}>
            <ListItemButton
              selected={index === 0}
              sx={{ minHeight: 60, px: 2 }}
            >
              <ListItemIcon sx={{ minWidth: 30 }}>
                {React.cloneElement(item.icon, { sx: { fontSize: 60 } })}
              </ListItemIcon>
              <ListItemText
                primary={<Typography sx={{ fontSize: 20 }}>{item.text}</Typography>}
              />
            </ListItemButton>
          </ListItem>
        ))}
      </List>

      {/* Secondary list */}
      <List dense sx={{ gap: 1.5 }}>
        {secondaryListItems.map((item, index) => (
          <ListItem key={index} disablePadding sx={{ display: 'block' }}>
            <ListItemButton sx={{ minHeight: 60, px: 2 }}>
              <ListItemIcon sx={{ minWidth: 30 }}>
                {React.cloneElement(item.icon, { sx: { fontSize: 60 } })}
              </ListItemIcon>
              <ListItemText
                primary={<Typography sx={{ fontSize: 20 }}>{item.text}</Typography>}
              />
            </ListItemButton>
          </ListItem>
        ))}
      </List>
    </Stack>
  );
}
