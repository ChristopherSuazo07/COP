import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import './index.css'
import App from './App.jsx'
import Dashboard from './dashboard/dashboard/Dashboard.jsx'
import Settings from './dashboard/dashboard/Settings.jsx'
import Notfoundpage from './hooks/NotFound.jsx'
import { createBrowserRouter, RouterProvider } from 'react-router-dom'
import View2 from './views/View2.jsx'
import View1 from './views/View1.jsx'
import View3 from './views/View3.jsx'


const router = createBrowserRouter([
  {
    path: '/',
    element: <App/>, // Layout del dashboard
    errorElement: <Notfoundpage />,
    children: [
      {
        path:'/',
        element:<View1/>
      },
      {
        path:'/AjustedeParametros',
        element:<View2/>
      },
      {
        path:'/Historial',
        element:<View3/>
      }

    ]
  }
])

createRoot(document.getElementById('root')).render(
  <StrictMode>
    <RouterProvider router={router} />
  </StrictMode>
)
