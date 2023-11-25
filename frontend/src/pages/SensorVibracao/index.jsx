import AlertasSensorVibracao from '../../components/AlertasSensorVibracao'
import "./index.scss"

function SensorVibracao() {
  return (
    <>
      <header>
        <h1>ConstruSensor - Alertas - Sensor de Vibracao</h1>
      </header>

      <main>
        <section id="container_sensor_vibracao">
          <AlertasSensorVibracao />
        </section>
      </main>
    </>
  )
}

export default SensorVibracao