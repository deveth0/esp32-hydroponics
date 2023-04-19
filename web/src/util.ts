import fetchRetry, { RequestInitWithRetry } from "fetch-retry";

export function apiFetch<T>(url: string, init?: RequestInitWithRetry): Promise<T> {
  const fetch = fetchRetry(global.fetch);
  return fetch(url, init).then(response => {
    if (!response.ok) {
      throw new Error(response.statusText);
    }
    return response.json() as Promise<T>;
  });
}

export function apiPostJson<T, U>(url: string, data: T): Promise<U> {
  return fetch(url, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(data),
  }).then(response => {
    if (!response.ok) {
      throw new Error(response.statusText);
    }
    return response.json() as Promise<U>;
  });
}
