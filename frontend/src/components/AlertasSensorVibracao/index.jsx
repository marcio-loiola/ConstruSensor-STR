import { Alert, AlertTitle } from "@mui/material"

function AlertasSensorVibracao() {
    return (
        <>
            <Alert severity="warning" className="alerta">
                <AlertTitle>Warning</AlertTitle>
                This is a warning alert — <strong>check it out!</strong>
            </Alert>

            <Alert severity="warning" className="alerta">
                <AlertTitle>Warning</AlertTitle>
                This is a warning alert — <strong>check it out!</strong>
            </Alert>
            
            <Alert severity="warning" className="alerta">
                <AlertTitle>Warning</AlertTitle>
                This is a warning alert — <strong>check it out!</strong>
            </Alert>
        </>
    )
}

export default AlertasSensorVibracao