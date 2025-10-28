import React, { useState } from "react";
import CircularProgress from "@mui/material/CircularProgress";
import Typography from "@mui/material/Typography";
import Box from "@mui/material/Box";
import Card from "@mui/material/Card";
import CardContent from "@mui/material/CardContent";

function CircularProgressWithLabel({ value, size = 120, text }) {
  const [hover, setHover] = useState(false);

  return (
    <Box
      sx={{ position: "relative", display: "inline-flex", cursor: "pointer" }}
      onMouseEnter={() => setHover(true)}
      onMouseLeave={() => setHover(false)}
    >
      <CircularProgress
        variant="determinate"
        value={value}
        size={size}
        thickness={4}
        sx={{
          color: hover
            ? "rgba(0, 204, 255, 1)" // Azul neón brillante
            : "rgba(0, 157, 255, 0.6)", // Azul neón base
          filter: hover
            ? "drop-shadow(0 0 20px rgba(0, 204, 255, 0.9))"
            : "drop-shadow(0 0 10px rgba(0, 153, 255, 0.6))",
          transition: "all 0.01s ease-in-out",
        }}
      />
      <Box
        sx={{
          top: 0,
          left: 0,
          bottom: 0,
          right: 0,
          position: "absolute",
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
        }}
      >
        <Typography
          variant="h5"
          component="div"
          sx={{ color: "text.primary", fontWeight: "bold" }}
        >
          {`${Math.round(value)}%`}
        </Typography>
      </Box>
    </Box>
  );
}

export default function CircularWithValueLabel({ porcentajetolva, text }) {
  return (
    <Card variant="outlined" sx={{ width: "100%" }}>
      <Typography component="h2" variant="subtitle1" gutterBottom sx={{marginBottom:"5%"}}>
          {text}
        </Typography>
      <CircularProgressWithLabel value={porcentajetolva} size={200} />
    </Card>
  );
}
