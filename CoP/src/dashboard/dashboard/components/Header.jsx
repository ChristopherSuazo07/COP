import * as React from 'react';
import Stack from '@mui/material/Stack';
import { useLocation } from 'react-router-dom';
import CustomDatePicker from './CustomDatePicker';
import NavbarBreadcrumbs from './NavbarBreadcrumbs';
import ColorModeIconDropdown from '../../../shared-theme/ColorModeIconDropdown';

export default function Header() {
  const location = useLocation();

  // Definimos los textos según la ruta
  const breadcrumbTextMap = {
    '/': 'Monitoreo de Proceso',
    '/AjustedeParametros': 'Ajustes de Parametros',
    '/Historial': 'Historial',
  };

  // Obtenemos el texto correspondiente, por defecto mostramos "/"
  const breadcrumbText = breadcrumbTextMap[location.pathname] || '';

  return (
    <Stack
      direction="row"
      sx={{
        display: { xs: 'none', md: 'flex' },
        width: '100%',
        alignItems: { xs: 'flex-start', md: 'center' },
        justifyContent: 'space-between',
        maxWidth: { sm: '100%', md: '1700px' },
        pt: 1.5,
      }}
      spacing={2}
    >
      <NavbarBreadcrumbs text={breadcrumbText} />
      <Stack direction="row" sx={{ gap: 1 }}>
        <CustomDatePicker />
        <ColorModeIconDropdown />
      </Stack>
    </Stack>
  );
}
