import { BrowserRouter as Router, Route, Routes } from 'react-router-dom'
import Home from './pages/Home'
import SensorRuido from './pages/SensorRuido'
import SensorTemperatura from './pages/SensorTemperatura'
import SensorVibracao from './pages/SensorVibracao'
import SensorPoeira from './pages/SensorPoeira'

function App() {
  return (
    <Router>
      <Routes>
        <Route path="/" element={<Home/>}/>
        <Route path="/sensor-ruido" element={<SensorRuido/>}/>
        <Route path="/sensor-temperatura" element={<SensorTemperatura/>}/>
        <Route path="/sensor-vibracao" element={<SensorVibracao/>}/>
        <Route path="/sensor-poeira" element={<SensorPoeira/>}/>
      </Routes>
    </Router>
  )
}

export default App
