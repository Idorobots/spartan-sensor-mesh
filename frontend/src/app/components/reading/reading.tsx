import * as preact from "preact";
import * as styles from "./reading.css";

interface Props {
  color: string;
  progress: number;
  isError: boolean;
  children?: Array<preact.Component<any, any>>;
}

export const Reading = (props: Props) => {
  const moreLess = (props.progress > 180) ? styles.progressMore : styles.progressLess;
  const leftStyle = {
    "border-color": props.color,
    "transform": "rotate(" + props.progress + "deg)",
    "-webkit-transform": "rotate(" + props.progress + "deg)",
    "-moz-transform": "rotate(" + props.progress + "deg)",
    "-ms-transform": "rotate(" + props.progress + "deg)"
  };
  const rightStyle = {
    "border-color": props.color
  };

  return (
    <div className={styles.wrapper + " " + moreLess}>
      <div className={styles.shadow}/>
      <div className={styles.gauge}>
        <div className={styles.left + " " + styles.progressBar} style={leftStyle}></div>
        <div className={styles.right + " " + styles.progressBar} style={rightStyle}></div>
      </div>
      <div className={props.isError ? styles.error : styles.hidden}/>
      <div className={styles.label}>
        {props.children}
      </div>
    </div>
  );
};
