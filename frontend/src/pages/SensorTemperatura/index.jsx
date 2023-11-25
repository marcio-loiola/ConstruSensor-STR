import AlertasSensorTemperatura from '../../components/AlertasSensorTemperatura'
import "./index.scss"

function SensorTemperatura() {
  return (
    <>
      <header>
        <h1>ConstruSensor - Alertas - Sensor de Temperatura</h1>
      </header>

      <main>
        <section id="container_sensor_temperatura">
          <AlertasSensorTemperatura />
        </section>
      </main>
    </>
  )
}

export default SensorTemperatura