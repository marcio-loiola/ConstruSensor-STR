import { Alert, AlertTitle } from "@mui/material"

function AlertasSensorRuido() {
    return (
        <>
            <Alert severity="warning" className="alerta">
                <AlertTitle>Warning</AlertTitle>
                This is a warning alert — <strong>check it out!</strong>
            </Alert>
        </>
    )
}

export default AlertasSensorRuido