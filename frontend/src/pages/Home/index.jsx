import { Badge, Card, CardContent } from '@mui/material'
import { useNavigate } from 'react-router-dom'
import "./index.scss"

function Home() {
  const history = useNavigate();

  return (
    <>
      <header>
        <h1>ConstruSensor - Sensores</h1>
      </header>

      <main>
        <section id="container_sensores">
          <Badge badgeContent={4} color="error">
            <Card className="sensor" onClick={() => { history("/sensor-ruido") }}>
              <CardContent className="conteudo">
                <span>Sensor de Ruído</span>
              </CardContent>
            </Card>
          </Badge>

          <Badge badgeContent={4} color="error">
            <Card className="sensor" onClick={() => { history("/sensor-temperatura") }}>
              <CardContent className="conteudo">
                <span>Sensor de Temperatura</span>
              </CardContent>
            </Card>
          </Badge>

          <Badge badgeContent={4} color="error">
            <Card className="sensor" onClick={() => { history("/sensor-vibracao") }}>
              <CardContent className="conteudo">
                <span>Sensor de Vibração</span>
              </CardContent>
            </Card>
          </Badge>

          <Badge badgeContent={4} color="error">
            <Card className="sensor" onClick={() => { history("/sensor-poeira") }}>
              <CardContent className="conteudo">
                <span>Sensor de Poeira</span>
              </CardContent>
            </Card>
          </Badge>
        </section>
      </main>
    </>
  )
}

export default Home