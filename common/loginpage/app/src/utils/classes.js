export function mergeClass(base, extra) {
  return {
    [base]: true,
    [extra]: !!extra,
  };
}
