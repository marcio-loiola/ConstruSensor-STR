import AlertasSensorPoeira from '../../components/AlertasSensorPoeira'
import "./index.scss"

function SensorPoeira() {
  return (
    <>
      <header>
        <h1>ConstruSensor - Alertas - Sensor de Poeira</h1>
      </header>

      <main>
        <section id="container_sensor_poeira">
          <AlertasSensorPoeira />
        </section>
      </main>
    </>
  )
}

export default SensorPoeira