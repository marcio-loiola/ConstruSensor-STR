import AlertasSensorRuido from '../../components/AlertasSensorRuido'
import "./index.scss"

function SensorRuido() {
  return (
    <>
      <header>
        <h1>ConstruSensor - Alertas - Sensor de Ruido</h1>
      </header>

      <main>
        <section id="container_sensor_ruido">
          <AlertasSensorRuido />
        </section>
      </main>
    </>
  )
}

export default SensorRuido