declare module "*.svg" {
  interface SVGIcon {
    content: string;
    id: string;
    viewBox: string;
  }

  const content: SVGIcon;
  export default content;
}
