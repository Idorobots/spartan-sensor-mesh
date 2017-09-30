import * as preact from "preact";
import * as styles from "./label.css";

interface LabelProps {
  name: string;
  value: number;
  unit: string;
  tooltip: string;
}

export const Label = (props: LabelProps) => (
  <div className={styles.labelWrapper}>
    <span className={styles.dummy}>{props.name}</span>
    <div title={props.tooltip} className={styles.valueWrapper}>
      <span className={styles.value}>{props.value}</span>
      <span className={styles.unit}>{props.unit}</span>
    </div>
    <span className={styles.name}>{props.name}</span>
  </div>
);
