import * as React from "react";
import { Link, useLocation } from "react-router-dom";
import List from "@mui/material/List";
import ListItem from "@mui/material/ListItem";
import ListItemButton from "@mui/material/ListItemButton";
import ListItemIcon from "@mui/material/ListItemIcon";
import ListItemText from "@mui/material/ListItemText";
import Stack from "@mui/material/Stack";
import Typography from "@mui/material/Typography";
import HomeRoundedIcon from "@mui/icons-material/HomeRounded";
import AnalyticsRoundedIcon from "@mui/icons-material/AnalyticsRounded";
import PeopleRoundedIcon from "@mui/icons-material/PeopleRounded";
import SettingsRoundedIcon from "@mui/icons-material/SettingsRounded";
import InfoRoundedIcon from "@mui/icons-material/InfoRounded";
import HelpRoundedIcon from "@mui/icons-material/HelpRounded";

const mainListItems = [
  { text: "Monitoreo", icon: <HomeRoundedIcon />, route: "/" },
  { text: "Ajuste de Variables", icon: <AnalyticsRoundedIcon />, route: "/AjustedeParametros" },
  { text: "Historial", icon: <PeopleRoundedIcon />, route: "/Historial" },
];

const secondaryListItems = [
  { text: "Settings", icon: <SettingsRoundedIcon /> },
  { text: "About", icon: <InfoRoundedIcon /> },
  { text: "Feedback", icon: <HelpRoundedIcon /> },
];

export default function MenuContent() {
  const location = useLocation(); // 👈 Detecta la ruta actual

  return (
    <Stack sx={{ flexGrow: 1, p: 1, justifyContent: "space-between" }}>
      {/* Main list */}
      <List dense sx={{ gap: 1.5 }}>
        {mainListItems.map((item, index) => (
          <ListItem key={index} disablePadding sx={{ display: "block" }}>
            <ListItemButton
              component={Link}
              to={item.route}
              selected={location.pathname === item.route} // 👈 Se marca automáticamente
              sx={{
                minHeight: 60,
                px: 2,
                color: "inherit",
                textDecoration: "none",
                "&.Mui-selected": {
                  backgroundColor: "rgba(25, 118, 210, 0.15)", // azul translúcido
                  color: "primary.main",
                  fontWeight: "bold",
                },
                "&.Mui-selected:hover": {
                  backgroundColor: "rgba(25, 118, 210, 0.25)",
                },
              }}
            >
              <ListItemIcon sx={{ minWidth: 30 }}>
                {React.cloneElement(item.icon, { sx: { fontSize: 40 } })}
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
          <ListItem key={index} disablePadding sx={{ display: "block" }}>
            <ListItemButton sx={{ minHeight: 60, px: 2 }}>
              <ListItemIcon sx={{ minWidth: 30 }}>
                {React.cloneElement(item.icon, { sx: { fontSize: 40 } })}
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
