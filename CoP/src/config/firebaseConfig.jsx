// src/firebaseConfig.jsx
import { initializeApp } from "firebase/app";
import { getDatabase } from "firebase/database";

// Import the functions you need from the SDKs you need
// TODO: Add SDKs for Firebase products that you want to use
// https://firebase.google.com/docs/web/setup#available-libraries

// Your web app's Firebase configuration
// For Firebase JS SDK v7.20.0 and later, measurementId is optional
const firebaseConfig = {
  apiKey: "AIzaSyCitW41T9jLgBGisdPkaR9BxKFvPnkeKW0",
  authDomain: "copcomcon.firebaseapp.com",
  databaseURL: "https://copcomcon-default-rtdb.firebaseio.com",
  projectId: "copcomcon",
  storageBucket: "copcomcon.firebasestorage.app",
  messagingSenderId: "51405257622",
  appId: "1:51405257622:web:2087b0378424bbdef3bbb7",
  measurementId: "G-EWEMCQ8N6V"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);

// Obtiene la referencia a la base de datos
const database = getDatabase(app);

// Función que exportamos para usar en React
export function getFirebaseDB() {
  return database;
}

export default app;
