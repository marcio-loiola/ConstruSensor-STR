import { Alert, AlertTitle, Badge, Box, Card, CardContent, Typography } from '@mui/material'
import { useState } from 'react'

function App() {
  return (
    <>
      <header>
        <h1>ConstruSensor - Sensores</h1>
      </header>

      <main>
        <section id="container_sensores">
          <Badge badgeContent={4} color="error">
            <Card className="sensor">
              <CardContent className="conteudo">
                <span>Sensor de Ruído</span>
              </CardContent>
            </Card>
          </Badge>

          <Badge badgeContent={4} color="error">
            <Card className="sensor">
              <CardContent className="conteudo">
                <span>Sensor de Temperatura</span>
              </CardContent>
            </Card>
          </Badge>

          <Badge badgeContent={4} color="error">
            <Card className="sensor">
              <CardContent className="conteudo">
                <span>Sensor de Vibração</span>
              </CardContent>
            </Card>
          </Badge>

          <Badge badgeContent={4} color="error">
            <Card className="sensor">
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

export default App
