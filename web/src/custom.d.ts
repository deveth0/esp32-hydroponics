declare module "*.svg" {
  interface SVGIcon {
    content: string;
    id: string;
    viewBox: string;
  }

  const content: SVGIcon;
  export default content;
}

declare module "*.png" {
  interface PNGImage {
    src: string;
  }

  const src: PNGImage;
  export default src;
}
