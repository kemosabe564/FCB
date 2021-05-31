# Commands

## 1 > Set/QueryMode
| 0001 XXXX | 8 BIT CRC |
  ID   MODE

MODE = 0000 -> Query current mode
MODE > 0000 -> Set Mode

## 2 < CurrentMode
| 0010 XXXX | 8 BIT CRC |
       MODE

## 3 > SetControl
| 0011 XXXX | XXXX XXXX | XXXX XXXX | XXXX XXXX | XXXX XXXX | 8 BIT CRC |
       MODE   YAW         PITCH       ROLL        CLIMB

MODE -> Control mode check
	If it doesn't correspond with current mode then 
	error

## 4 < AckControl
| 0100 XXXX | 8 BIT CRC |
       ACK
       
ACK = 0000 -> Ack success
ACK = 0001 -> Wrong flightmode 

## 5 > QueryForces
| 0101 0000 | 8 BIT CRC |

## 6 < CurrentTelemetry
| 0110 0000 | PHI, THETA, PSI,  | 8 BIT CRC |

## 7 < DebugMsg
| 0111 XXXX | XXXX XXXX | ........ | 8 BIT CRC |
       ID     N
       
ID -> Identifying location
N -> 4 bit for message length in * 8 characters. Starts at 1 up to 17 since 0 size is nonsensical


## 8 > SetParam
| 1000 XXXX | XXXX XXXX | 8 BIT CRC |
       PID    PARAM VAL
       
PID -> 4 bit ID. Might not be enough
       
PID -> Param ID
## 9 < AckParam
| 1001 XXXX | 8 BIT CRC |
       PID
       
PID -> Param ID

## 10 <> Heartbeat
| 1010 XXXX | 8 BIT CRC |
       SEQ

SEQ -> Sequence number. Increments every time a heartbeat is sent and cycles around after 15.

Heartbeat is initiated by the Terminal and the drone responds in best-effort time with the current sequence number.

## Extended
| 1111 XXXX | xxxx




